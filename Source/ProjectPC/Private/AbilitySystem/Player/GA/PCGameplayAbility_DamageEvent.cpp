// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Player/GA/PCGameplayAbility_DamageEvent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"

#include "BaseGameplayTags.h"


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

	auto Damage = TriggerEventData->EventMagnitude;
	auto Instigator = TriggerEventData->Instigator.Get();
	auto Target = const_cast<AActor*>(TriggerEventData->Target.Get());

	if (!Instigator || !Target)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (auto TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target))
	{
		FGameplayEffectSpecHandle HPSpecHandle = MakeOutgoingGameplayEffectSpec(GE_PlayerHPChange, GetAbilityLevel());
		if (HPSpecHandle.IsValid())
		{
			HPSpecHandle.Data->SetSetByCallerMagnitude(PlayerGameplayTags::Player_Stat_PlayerHP, -Damage);
			TargetASC->ApplyGameplayEffectSpecToSelf(*HPSpecHandle.Data.Get());
		}
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UPCGameplayAbility_DamageEvent::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
