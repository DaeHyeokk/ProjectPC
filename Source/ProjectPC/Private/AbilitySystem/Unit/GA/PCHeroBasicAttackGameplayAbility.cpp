// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Unit/GA/PCHeroBasicAttackGameplayAbility.h"

#include "AbilitySystem/Unit/AttributeSet/PCHeroUnitAttributeSet.h"


bool UPCHeroBasicAttackGameplayAbility::CheckCost(const FGameplayAbilitySpecHandle Handle,
                                                  const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!ActorInfo || !ActorInfo->AbilitySystemComponent.IsValid())
		return false;

	// Cost Gameplay Effect Class가 None일 경우 True
	if (!CostGameplayEffectClass)
		return true;

	const UPCHeroUnitAttributeSet* HeroAttributeSet = ActorInfo->AbilitySystemComponent->GetSet<UPCHeroUnitAttributeSet>();
	if (!HeroAttributeSet)
		return false;
	
	const float CurrentValue = CostGameplayAttribute.GetGameplayAttributeData(HeroAttributeSet)->GetCurrentValue();
	return CurrentValue >= CostValue;
}

void UPCHeroBasicAttackGameplayAbility::ApplyCost(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	if (!CostGameplayEffectClass || !ActorInfo || !ActorInfo->AbilitySystemComponent.IsValid())
	{
		return;
	}

	FGameplayEffectSpecHandle CostSpec = MakeOutgoingGameplayEffectSpec(CostGameplayEffectClass, GetAbilityLevel());
	if (CostSpec.IsValid())
	{
		CostSpec.Data->SetSetByCallerMagnitude(CostEffectCallerTag, -CostValue);
		(void)ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, CostSpec);
	}
}
