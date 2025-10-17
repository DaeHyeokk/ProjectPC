// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Unit/GA/Synergy/PCSynergyBladeMasterGameplayAbility.h"

#include "BaseGameplayTags.h"


UPCSynergyBladeMasterGameplayAbility::UPCSynergyBladeMasterGameplayAbility()
{
	AbilityTags.AddTag(UnitGameplayTags::Unit_Ability_Synergy_BladeMaster);
	WaitEventTag = UnitGameplayTags::Unit_Event_HitSucceed_Basic;

	ChancePercentByLevel.Add(45.f);
}

TArray<FActiveGameplayEffectHandle> UPCSynergyBladeMasterGameplayAbility::ApplyCommittedEffectSpec(
	UAbilitySystemComponent* ASC, const AActor* Target)
{
	TArray<FActiveGameplayEffectHandle> ApplyEffectHandles;
	
	if (!HasAuthority(&CurrentActivationInfo) || !ASC)
		return ApplyEffectHandles;

	int BonusAttackCnt = static_cast<int>(BonusAttackCount.GetValueAtLevel(GetAbilityLevel()));
	const FPCEffectSpecList* List = &AbilityConfig.OnCommittedEffectSpecs;

	while (BonusAttackCnt--)
	{
		ApplyEffectSpec(List, ASC, Target);
	}
	
	return ApplyEffectHandles;
}
