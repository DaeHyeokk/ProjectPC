// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Unit/GA/PCUnitCombatWinGameplayAbility.h"

#include "BaseGameplayTags.h"

UPCUnitCombatWinGameplayAbility::UPCUnitCombatWinGameplayAbility()
{
	ActivationBlockedTags.AddTag(UnitGameplayTags::Unit_State_Combat_Dead);

	FAbilityTriggerData TriggerData;;
	TriggerData.TriggerTag = UnitGameplayTags::Unit_Event_Combat_Win;
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(TriggerData);
}

void UPCUnitCombatWinGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	// 서버가 아니거나 Commit에 실패했을 경우 어빌리티 종료
	if (!HasAuthority(&ActivationInfo) || !CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, false, true);
		return;
	}
}