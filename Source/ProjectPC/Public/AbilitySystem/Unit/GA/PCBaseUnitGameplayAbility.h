// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "PCBaseUnitGameplayAbility.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCBaseUnitGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UPCBaseUnitGameplayAbility();

protected:
	virtual bool CheckCost(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

	virtual void ApplyCost(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo) const override;
	
	virtual void ApplyCooldown(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo) const override;

	// virtual void EndAbility(
	// 	const FGameplayAbilitySpecHandle Handle,
	// 	const FGameplayAbilityActorInfo* ActorInfo,
	// 	const FGameplayAbilityActivationInfo ActivationInfo,
	// 	bool bReplicateEndAbility, bool bWasCancelled) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Costs)
	FGameplayTag CostEffectCallerTag;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Costs)
	float CostValue = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Costs)
	FGameplayAttribute CostGameplayAttribute;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Cooldowns)
	FGameplayTag CooldownEffectCallerTag;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Cooldowns)
	float CooldownDuration = 0.f;
};
