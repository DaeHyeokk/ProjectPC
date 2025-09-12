// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseGameplayTags.h"
#include "AbilitySystem/Unit/GA/PCBaseUnitGameplayAbility.h"
#include "PCUnitBaseAttackGameplayAbility.generated.h"

class UPCDataAsset_UnitAnimSet;
class UPCUnitAttributeSet;
/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCUnitBaseAttackGameplayAbility : public UPCBaseUnitGameplayAbility
{
	GENERATED_BODY()

public:
	UPCUnitBaseAttackGameplayAbility();
	
protected:
	virtual void OnAvatarSet(
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilitySpec& Spec) override;
	
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual UAnimMontage* GetMontage(const FGameplayAbilityActorInfo* ActorInfo) const { return nullptr; }
	
	UPROPERTY(Transient)
	TObjectPtr<const UPCUnitAttributeSet> UnitAttrSet;

	UPROPERTY(Transient)
	TObjectPtr<const UPCDataAsset_UnitAnimSet> UnitAnimSet;
	
protected:
	virtual void ApplyGameplayEffect() { }
	
	// UFUNCTION()
	// virtual void OnAttackCommit(FGameplayEventData Payload);
	UFUNCTION()
	void OnMontageCompleted();
	UFUNCTION()
	void OnMontageCancelled();
	UFUNCTION()
	void OnMontageBlendOut();
	UFUNCTION()
	void OnMontageInterrupted();
};
