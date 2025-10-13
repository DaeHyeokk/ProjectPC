// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Player/GA/PCGameplayAbility_DamageEvent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Character.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"

#include "BaseGameplayTags.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Character/Player/PCPlayerCharacter.h"
#include "GameFramework/WorldSubsystem/PCProjectilePoolSubsystem.h"


UPCGameplayAbility_DamageEvent::UPCGameplayAbility_DamageEvent()
{
	AbilityTags.AddTag(PlayerGameplayTags::Player_Event_Damage);

	ActivationRequiredTags.AddTag(PlayerGameplayTags::Player_State_Normal);

	ActivationBlockedTags.AddTag(PlayerGameplayTags::Player_State_Dead);
	ActivationBlockedTags.AddTag(PlayerGameplayTags::Player_State_Carousel);

	FAbilityTriggerData TriggerData;;
	TriggerData.TriggerTag = PlayerGameplayTags::Player_Event_Damage;
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(TriggerData);
}

bool UPCGameplayAbility_DamageEvent::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}
	
	if (!ActorInfo->IsNetAuthority())
	{
		return false;
	}
	
	return true;
}

void UPCGameplayAbility_DamageEvent::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                     const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                                     const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!TriggerEventData)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	Damage = TriggerEventData->EventMagnitude;
	TargetPS = const_cast<AActor*>(TriggerEventData->Target.Get());
	auto InstigatorPS = const_cast<AActor*>(TriggerEventData->Instigator.Get());

	if (!InstigatorPS || !TargetPS.Get())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	APawn* InstigatorPawn = nullptr;
	APawn* TargetPawn = nullptr;

	FGameplayTag CharacterTag;
	FGameplayTag AttackTypeTag;
	
	if (auto InstigatorASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(InstigatorPS))
	{
		InstigatorPawn = Cast<APawn>(InstigatorASC->GetAvatarActor());

		FGameplayTagContainer InstigatorTags;
		InstigatorASC->GetOwnedGameplayTags(InstigatorTags);

		for (const FGameplayTag& Tag : InstigatorTags)
		{
			if (Tag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("Player.Type"))))
			{
				CharacterTag = Tag;
			}
			else if (Tag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("Player.Action.Attack"))))
			{
				AttackTypeTag = Tag;
			}
		}
	}
	
	if (auto TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetPS.Get()))
	{
		TargetPawn = Cast<APawn>(TargetASC->GetAvatarActor());
	}
	
	if (!InstigatorPawn || !TargetPawn)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (PlayerAttackAnimData)
	{
		if (auto AttackMontageSoftPtr = PlayerAttackAnimData->GetAttackMontage(CharacterTag, AttackTypeTag))
		{
			FSoftObjectPath MontagePath = AttackMontageSoftPtr.ToSoftObjectPath();
			FStreamableManager& Streamable = UAssetManager::GetStreamableManager();

			TWeakObjectPtr<UPCGameplayAbility_DamageEvent> WeakThis = this;
			TWeakObjectPtr<APawn> WeakInstigatorPawn = InstigatorPawn;
			
			Streamable.RequestAsyncLoad(MontagePath, [WeakThis, MontagePath, WeakInstigatorPawn]()
			{
				if (WeakThis.IsValid())
				{
					if (UAnimMontage* LoadedMontage = Cast<UAnimMontage>(MontagePath.ResolveObject()))
					{
						if (auto InstigatorChar = Cast<ACharacter>(WeakInstigatorPawn))
						{
							InstigatorChar->PlayAnimMontage(LoadedMontage);

							if (auto* PCChar = Cast<APCPlayerCharacter>(InstigatorChar))
							{
								PCChar->Client_PlayMontage(LoadedMontage, 1.f);
							}
						}
					}
				}
			});
		}
	}

	if (auto* ProjectilePoolSubsystem = GetWorld()->GetSubsystem<UPCProjectilePoolSubsystem>())
	{
		FVector InstLoc = InstigatorPawn->GetActorLocation();
		FVector TargetLoc = TargetPawn->GetActorLocation();
		FVector Direction = (TargetLoc - InstLoc).GetSafeNormal();

		FTransform SpawnTransform(Direction.Rotation(), InstLoc);
		ProjectilePoolSubsystem->SpawnProjectile(SpawnTransform, CharacterTag, AttackTypeTag, InstigatorPawn, TargetPawn, true);
	}
	
	auto WaitHit = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, PlayerGameplayTags::Player_Event_ProjectileHit);
	WaitHit->EventReceived.AddDynamic(this, &UPCGameplayAbility_DamageEvent::OnProjectileHitEvent);
	WaitHit->ReadyForActivation();
}

void UPCGameplayAbility_DamageEvent::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	TargetPS = nullptr;
	Damage = 0.f;
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UPCGameplayAbility_DamageEvent::OnProjectileHitEvent(FGameplayEventData EventData)
{
	if (auto TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetPS.Get()))
	{
		FGameplayEffectSpecHandle HPSpecHandle = MakeOutgoingGameplayEffectSpec(GE_PlayerHPChange, GetAbilityLevel());
		if (HPSpecHandle.IsValid())
		{
			HPSpecHandle.Data->SetSetByCallerMagnitude(PlayerGameplayTags::Player_Stat_PlayerHP, -Damage);
			TargetASC->ApplyGameplayEffectSpecToSelf(*HPSpecHandle.Data.Get());
		}
	}

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}