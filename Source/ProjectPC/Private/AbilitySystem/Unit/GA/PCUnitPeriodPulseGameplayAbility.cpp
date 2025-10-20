// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Unit/GA/PCUnitPeriodPulseGameplayAbility.h"

#include "BaseGameplayTags.h"
#include "AbilitySystem/Unit/EffectSpec/PCEffectSpec.h"
#include "Character/Unit/PCBaseUnitCharacter.h"
#include "Tasks/GameplayTask_WaitDelay.h"


UPCUnitPeriodPulseGameplayAbility::UPCUnitPeriodPulseGameplayAbility()
{
	AbilityTags.AddTag(UnitGameplayTags::Unit_Ability_PeriodPulse);
}

void UPCUnitPeriodPulseGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!HasAuthority(&ActivationInfo) || !ActorInfo)
		return;
	
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			PulseTimer, this, &ThisClass::OnPulseTick, PeriodSeconds, true, PeriodSeconds);
	}
}

void UPCUnitPeriodPulseGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	if (PulseTimer.IsValid())
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(PulseTimer);
		}
		PulseTimer.Invalidate();
	}
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UPCUnitPeriodPulseGameplayAbility::OnPulseTick()
{
	if (!RollChance())
		return;

	ApplyConfiguredEffects();
}

void UPCUnitPeriodPulseGameplayAbility::ApplyConfiguredEffects()
{
	TArray<FActiveGameplayEffectHandle> OutHandles;
	
	UAbilitySystemComponent* ASC = Unit->GetAbilitySystemComponent();
	if (!ASC)
		return;
	
	if (!CommitAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo))
		return;
	
	ApplyCommittedEffectSpec(ASC);
}
