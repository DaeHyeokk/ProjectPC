// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Unit/GA/PCUnitBasicAttackGameplayAbility.h"
#include "PCHeroBasicAttackGameplayAbility.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCHeroBasicAttackGameplayAbility : public UPCUnitBasicAttackGameplayAbility
{
	GENERATED_BODY()
	
public:
	UPCHeroBasicAttackGameplayAbility();

protected:
	virtual bool CheckCost(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

	virtual void ApplyCost(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo) const override;
	
	
};
