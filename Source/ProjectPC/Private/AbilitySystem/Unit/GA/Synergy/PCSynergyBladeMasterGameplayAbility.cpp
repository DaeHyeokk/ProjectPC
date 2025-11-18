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

	int8 BonusAttackCnt = static_cast<int8>(BonusAttackCount.GetValueAtLevel(GetAbilityLevel()));
	const FPCEffectSpecList* List = &AbilityConfig.OnCommittedEffectSpecs;

	while (BonusAttackCnt--)
	{
		for (const FActiveGameplayEffectHandle& EffectHandle : ApplyEffectSpec(List, ASC, Target))
		{
			if (EffectHandle.IsValid())
			{
				ApplyEffectHandles.Add(EffectHandle);
			}
		}
	}
	
	return ApplyEffectHandles;
}
