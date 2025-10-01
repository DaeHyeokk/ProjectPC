// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Player/GA/PCGameplayAbility_DamageEvent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"

#include "BaseGameplayTags.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
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

	if (auto* ProjectilePoolSubsystem = GetWorld()->GetSubsystem<UPCProjectilePoolSubsystem>())
	{
		APawn* InstigatorPawn = nullptr;
		APawn* TargetPawn = nullptr;
		
		if (auto InstigatorASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(InstigatorPS))
		{
			InstigatorPawn = Cast<APawn>(InstigatorASC->GetAvatarActor());
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
		
		FVector InstLoc = InstigatorPawn->GetActorLocation();
		FVector TargetLoc = TargetPawn->GetActorLocation();
		FVector Direction = (TargetLoc - InstLoc).GetSafeNormal();

		FTransform SpawnTransform(Direction.Rotation(), InstLoc);
		ProjectilePoolSubsystem->SpawnProjectile(SpawnTransform, PlayerGameplayTags::Player_Type_LittleDragon, PlayerGameplayTags::Player_Action_Attack_Basic, InstigatorPawn, TargetPawn, true);
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