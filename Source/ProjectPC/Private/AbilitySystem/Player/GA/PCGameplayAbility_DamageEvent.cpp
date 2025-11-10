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
#include "Character/Projectile/PCBaseProjectile.h"
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

	// 서버 권위면 Activate
	if (ActorInfo && ActorInfo->IsNetAuthority())
	{
		return true;
	}
	
	return false;
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

	// GA 이벤트 호출 시, 받은 TriggerEventData를 통해 필요한 데이터 분해 
	float Damage = TriggerEventData->EventMagnitude;
	TWeakObjectPtr<AActor> TargetPS = const_cast<AActor*>(TriggerEventData->Target.Get());
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

	// 발사체 활성화를 위한 캐릭터 Tag, 공격타입 Tag 얻기
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

	// 애님 몽타주 실행 (비동기 로드)
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

	// 발사체 활성화
	if (auto* ProjectilePoolSubsystem = GetWorld()->GetSubsystem<UPCProjectilePoolSubsystem>())
	{
		FVector InstLoc = InstigatorPawn->GetActorLocation();
		FVector TargetLoc = TargetPawn->GetActorLocation();
		FVector Direction = (TargetLoc - InstLoc).GetSafeNormal();

		FTransform SpawnTransform(Direction.Rotation(), InstLoc);
		auto Projectile = ProjectilePoolSubsystem->SpawnProjectile(SpawnTransform, CharacterTag, InstigatorPawn, TargetPawn);
		Projectile->SetDamage(-Damage);
	}
}

void UPCGameplayAbility_DamageEvent::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}