// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Unit/GA/PCUnitWaitEventGameplayAbility.h"

#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Character/Unit/PCBaseUnitCharacter.h"

void UPCUnitWaitEventGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                      const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                                      const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!HasAuthority(&ActivationInfo) || !ActorInfo)
		return;

	StartWaitTask();
}

void UPCUnitWaitEventGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	if (WaitTask)
	{
		WaitTask->EndTask();
		WaitTask = nullptr;
	}
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UPCUnitWaitEventGameplayAbility::HandleEventReceived(const FGameplayEventData& Payload)
{
	if (!RollChance() || !ShouldCommitOnEvent(Payload))
	{
		return;
	}
	
	if (!CommitAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo))
	{
		return;
	}

	const AActor* Target = Payload.Target;
	const AActor* Avatar = GetAvatarActorFromActorInfo();
	UAbilitySystemComponent* ASC = Unit->GetAbilitySystemComponent();

	if (!Target || !Avatar || !ASC)
	{
		return;
	}
	
	ApplyCommittedEffectSpec(ASC, Target);
}

void UPCUnitWaitEventGameplayAbility::OnEventReceived(FGameplayEventData Payload)
{
	HandleEventReceived(Payload);

	if (bOnlyTriggerOnce && WaitTask)
	{
		WaitTask->EndTask();
		WaitTask = nullptr;
	}
}

void UPCUnitWaitEventGameplayAbility::StartWaitTask()
{
	if (!WaitEventTag.IsValid())
		return;

	if (WaitTask)
	{
		WaitTask->EndTask();
		WaitTask = nullptr;
	}

	WaitTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this, WaitEventTag, nullptr, bOnlyTriggerOnce, bOnlyMatchExact);

	if (!WaitTask)
		return;

	WaitTask->EventReceived.AddDynamic(this, &ThisClass::OnEventReceived);
	WaitTask->ReadyForActivation();
}
