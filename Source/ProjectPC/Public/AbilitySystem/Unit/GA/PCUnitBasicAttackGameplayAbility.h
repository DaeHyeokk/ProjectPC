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
	virtual void OnAvatarSet(
		const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	
	virtual void ApplyCooldown(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo) const override;
	
	virtual UAnimMontage* GetMontage(const FGameplayAbilityActorInfo* ActorInfo) const override;
	virtual float GetMontagePlayRate(const UAnimMontage* Montage) override;
	
	virtual void ApplyGameplayEffect() override;

	UPROPERTY(Transient)
	TSubclassOf<UGameplayEffect> ManaGainEffectClass;

	float ManaGainAmount = 10.f;
};
