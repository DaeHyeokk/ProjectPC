// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Unit/GA/PCUnitPassiveGameplayAbility.h"
#include "PCUnitWaitEventGameplayAbility.generated.h"

class UAbilityTask_WaitGameplayEvent;
/**
 * 
 */
UCLASS(Abstract)
class PROJECTPC_API UPCUnitWaitEventGameplayAbility : public UPCUnitPassiveGameplayAbility
{
	GENERATED_BODY()

protected:
	virtual void ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility, bool bWasCancelled) override;
	
	virtual void HandleEventReceived(const FGameplayEventData& Payload);
	
	UFUNCTION(BlueprintNativeEvent, Category="WaitEvent|Rule")
	bool ShouldCommitOnEvent(const FGameplayEventData& Payload);
	virtual bool ShouldCommitOnEvent_Implementation(const FGameplayEventData& Payload) { return true; }
	
	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitGameplayEvent> WaitTask = nullptr;

	UPROPERTY(EditDefaultsOnly, Category="Event")
	FGameplayTag WaitEventTag;
	
	UPROPERTY(EditDefaultsOnly, Category="Event")
	bool bOnlyTriggerOnce = false;

	UPROPERTY(EditDefaultsOnly, Category="Event")
	bool bOnlyMatchExact = true;
	
private:
	UFUNCTION()
	void OnEventReceived(FGameplayEventData Payload);
	
	void StartWaitTask();
};
