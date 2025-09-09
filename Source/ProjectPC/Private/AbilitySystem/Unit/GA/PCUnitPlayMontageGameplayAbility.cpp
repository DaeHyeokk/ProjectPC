// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Unit/GA/PCUnitPlayMontageGameplayAbility.h"

#include "BaseGameplayTags.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AnimNodes/AnimNode_RandomPlayer.h"

UPCUnitPlayMontageGameplayAbility::UPCUnitPlayMontageGameplayAbility()
{
	
	ActivationBlockedTags.AddTag(UnitGameplayTags::Unit_State_Combat_Death);
}

void UPCUnitPlayMontageGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (HasAuthority(&ActivationInfo))
	{
		if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
			return;
		}
		
		if (UAnimMontage* Montage = GetMontage(ActorInfo))
		{
			UAbilityTask_PlayMontageAndWait* Task = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
				this, NAME_None, Montage, 1.f);

			Task->OnCompleted.AddDynamic(this, &ThisClass::OnMontageCompleted);
			Task->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageCompleted);
			Task->OnBlendOut.AddDynamic(this, &ThisClass::OnMontageCompleted);
			Task->OnCancelled.AddDynamic(this, &ThisClass::OnMontageCompleted);
			Task->ReadyForActivation();
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Unit Ability Montage Nullptr"));
			EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
		}
	}
}

void UPCUnitPlayMontageGameplayAbility::PlayMontageAndWait()
{
}

void UPCUnitPlayMontageGameplayAbility::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
}
