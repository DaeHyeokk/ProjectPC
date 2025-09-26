// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Unit/GA/PCUnitDeathGameplayAbility.h"

#include "BaseGameplayTags.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"


UPCUnitDeathGameplayAbility::UPCUnitDeathGameplayAbility()
{
	// CancelAbilitiesWithTag.AddTag(UnitGameplayTags::Unit_Action);
	//
	// FAbilityTriggerData Trigger;
	// Trigger.TriggerSource = EGameplayAbilityTriggerSource::OwnedTagAdded;
	// Trigger.TriggerTag = UnitGameplayTags::Unit_State_Combat_Dead;
	// AbilityTriggers.Add(Trigger);
}

void UPCUnitDeathGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!Unit || HasAuthority(&ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, false, true);
		return;
	}

	if (UAnimMontage* Montage = MontageConfig.Montage)
	{
		UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this, NAME_None, Montage, 1.f, NAME_None, false);
	
		 //MontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageFinished);
		// MontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageFinished);
		// MontageTask->OnBlendOut.AddDynamic(this, &ThisClass::OnMontageFinished);
		// MontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageFinished);
		MontageTask->ReadyForActivation();
	}
}
