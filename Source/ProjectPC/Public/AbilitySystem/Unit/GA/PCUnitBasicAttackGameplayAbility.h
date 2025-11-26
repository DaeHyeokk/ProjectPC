// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Unit/GA/PCUnitBaseAttackGameplayAbility.h"
#include "PCUnitBasicAttackGameplayAbility.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCUnitBasicAttackGameplayAbility : public UPCUnitBaseAttackGameplayAbility
{
	GENERATED_BODY()
	
public:
	UPCUnitBasicAttackGameplayAbility();

protected:
	virtual void ApplyCooldown(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo) const override;
	
	virtual void ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData) override;
	
	virtual void SetMontage() override;
	virtual FGameplayTag GetMontageTag() override { return UnitGameplayTags::Unit_Montage_Attack_Basic; }
	virtual float GetMontagePlayRate() override;

	virtual void OnAttackSucceed(FGameplayEventData Payload) override;
	bool bAttackSucceed = false;
};
