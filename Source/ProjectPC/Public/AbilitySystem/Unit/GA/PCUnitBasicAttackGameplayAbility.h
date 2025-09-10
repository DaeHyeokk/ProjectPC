// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseGameplayTags.h"
#include "AbilitySystem/Unit/GA/PCUnitPlayMontageGameplayAbility.h"
#include "PCUnitBasicAttackGameplayAbility.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCUnitBasicAttackGameplayAbility : public UPCUnitPlayMontageGameplayAbility
{
	GENERATED_BODY()
	
protected:
	// virtual bool CheckCost(
	// const FGameplayAbilitySpecHandle Handle,
	// const FGameplayAbilityActorInfo* ActorInfo,
	// FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	//
	// virtual void ApplyCost(
	// 	const FGameplayAbilitySpecHandle Handle,
	// 	const FGameplayAbilityActorInfo* ActorInfo,
	// 	const FGameplayAbilityActivationInfo ActivationInfo) const override;
	
	virtual void OnGiveAbility(
		const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	
	virtual void ApplyCooldown(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo) const override;
	
	virtual UAnimMontage* GetMontage(const FGameplayAbilityActorInfo* ActorInfo) const override;

//	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Cooldowns)
//	FGameplayTag CooldownEffectCallerTag;
};
