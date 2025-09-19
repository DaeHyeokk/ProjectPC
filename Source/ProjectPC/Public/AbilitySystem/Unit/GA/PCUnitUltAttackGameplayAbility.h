// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Unit/GA/PCUnitBaseAttackGameplayAbility.h"
#include "PCUnitUltAttackGameplayAbility.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCUnitUltAttackGameplayAbility : public UPCUnitBaseAttackGameplayAbility
{
	GENERATED_BODY()

public:
	UPCUnitUltAttackGameplayAbility();
	
protected:
	virtual bool CheckCost(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	
	virtual void ApplyCost(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo) const override;
	
	virtual FGameplayTag GetMontageTag() override { return UnitGameplayTags::Unit_Montage_Attack_Ultimate; }
};
