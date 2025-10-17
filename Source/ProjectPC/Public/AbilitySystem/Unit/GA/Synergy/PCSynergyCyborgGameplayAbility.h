// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseGameplayTags.h"
#include "AbilitySystem/Unit/GA/PCUnitPeriodPulseGameplayAbility.h"
#include "PCSynergyCyborgGameplayAbility.generated.h"

class UAbilityTask_WaitGameplayEvent;
/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCSynergyCyborgGameplayAbility : public UPCUnitPeriodPulseGameplayAbility
{
	GENERATED_BODY()
	
public:
	UPCSynergyCyborgGameplayAbility();

protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility, bool bWasCancelled) override;

	virtual TArray<FActiveGameplayEffectHandle> ApplyCommittedEffectSpec(
		UAbilitySystemComponent* ASC, const AActor* Target = nullptr) override;
	
	UPROPERTY(EditDefaultsOnly, Category="Cyborg|Config", meta=(ClampMin="1"))
	int32 MaxStacks = 8;

	UPROPERTY(EditDefaultsOnly, Category="Cyborg|FullStackData")
	FScalableFloat BonusTrueDamagePercentage;
	
	FGameplayTag DamageAppliedEventTag = UnitGameplayTags::Unit_Event_DamageApplied;
	
	int32 CurrentStack = 0;
	TArray<FActiveGameplayEffectHandle> ActivePerStackEffectHandles;

private:
	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitGameplayEvent> WaitTask = nullptr;
	bool bIsListening = false;
	
	void RevokeAllPerStackEffects();

	void StartDamageAppliedEventWaitTask();
	void StopDamageAppliedEventWaitTask();

	UFUNCTION()
	void OnDamageAppliedEventReceived(FGameplayEventData Payload);
};
