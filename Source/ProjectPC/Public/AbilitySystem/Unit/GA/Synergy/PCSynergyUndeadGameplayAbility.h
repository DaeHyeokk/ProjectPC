// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Unit/GA/PCUnitWaitEventGameplayAbility.h"
#include "PCSynergyUndeadGameplayAbility.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCSynergyUndeadGameplayAbility : public UPCUnitWaitEventGameplayAbility
{
	GENERATED_BODY()

public:
	UPCSynergyUndeadGameplayAbility();
	
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
	
	virtual TArray<FActiveGameplayEffectHandle> ApplyCommittedEffectSpec(UAbilitySystemComponent* ASC, const AActor* Target = nullptr) override;
	virtual bool ShouldCommitOnEvent_Implementation(const FGameplayEventData& Payload) override;

private:
	void RevokeAllCachedEffects();
	
	TArray<FActiveGameplayEffectHandle> CachedEffectHandles;
	bool bAttackSpeedApplied = false;
};
