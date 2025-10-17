// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Unit/GA/PCUnitUltAttackGameplayAbility.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/Unit/AttributeSet/PCHeroUnitAttributeSet.h"
#include "Character/Unit/PCBaseUnitCharacter.h"


UPCUnitUltAttackGameplayAbility::UPCUnitUltAttackGameplayAbility()
{
	AbilityTags.AddTag(UnitGameplayTags::Unit_Ability_Attack_Ultimate);
}

bool UPCUnitUltAttackGameplayAbility::CheckCost(const FGameplayAbilitySpecHandle Handle,
                                                const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const
	{
	if (!ActorInfo || !ActorInfo->AbilitySystemComponent.IsValid())
		return false;

	const UAbilitySystemComponent* ASC = Unit ? Unit->GetAbilitySystemComponent() : nullptr;
	
	if (!CostGameplayEffectClass && !ASC)
		return false;

	const float UltCost = ASC->GetNumericAttribute(UPCHeroUnitAttributeSet::GetUltimateCostAttribute());

	const float CurrentValue = ASC->GetNumericAttribute(AbilityConfig.CostGameplayAttribute);

	return CurrentValue >= UltCost;
}

void UPCUnitUltAttackGameplayAbility::ApplyCost(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	if (!ActorInfo || !ActorInfo->AbilitySystemComponent.IsValid())
		return;

	UAbilitySystemComponent* ASC = Unit ? Unit->GetAbilitySystemComponent() : nullptr;
	if (!CostGameplayEffectClass || !ASC)
		return;
	
	const float UltCost = ASC->GetNumericAttribute(UPCHeroUnitAttributeSet::GetUltimateCostAttribute());
		
	FGameplayEffectSpecHandle CostSpec = MakeOutgoingGameplayEffectSpec(CostGameplayEffectClass, GetAbilityLevel());
	if (CostSpec.IsValid())
	{
		const FGameplayTag CostCallerTag = AbilityConfig.CostCallerTag;
			
		CostSpec.Data->SetSetByCallerMagnitude(CostCallerTag, -UltCost);
		ASC->ApplyGameplayEffectSpecToSelf(*CostSpec.Data.Get());
	}
}