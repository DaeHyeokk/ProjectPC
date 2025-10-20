// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Unit/GA/PCUnitPassiveGameplayAbility.h"
#include "PCUnitPeriodPulseGameplayAbility.generated.h"

class UGameplayTask_WaitDelay;
/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCUnitPeriodPulseGameplayAbility : public UPCUnitPassiveGameplayAbility
{
	GENERATED_BODY()
	
public:
	UPCUnitPeriodPulseGameplayAbility();

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
	
	UPROPERTY(EditDefaultsOnly, Category="Pulse", meta=(ClampMin="0.01", UIMin="0.01"))
	float PeriodSeconds = 3.f;

	virtual void ApplyConfiguredEffects();
	UFUNCTION()
	virtual void OnPulseTick();

	UPROPERTY(Transient)
	TObjectPtr<UGameplayTask_WaitDelay> WaitDelayTask = nullptr;
	
private:
	FTimerHandle PulseTimer;
};
