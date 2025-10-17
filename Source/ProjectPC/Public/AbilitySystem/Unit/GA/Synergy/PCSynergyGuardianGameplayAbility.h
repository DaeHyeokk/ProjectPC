// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseGameplayTags.h"
#include "AbilitySystem/Unit/GA/PCBaseUnitGameplayAbility.h"
#include "PCSynergyGuardianGameplayAbility.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCSynergyGuardianGameplayAbility : public UPCBaseUnitGameplayAbility
{
	GENERATED_BODY()
	
public:
	UPCSynergyGuardianGameplayAbility();

protected:
	virtual void OnGiveAbility(
		const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;

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

	
	FGameplayTag GuardianSynergyTag = SynergyGameplayTags::Synergy_Job_Guardian;

private:
	TMap<TWeakObjectPtr<UAbilitySystemComponent>, TArray<FActiveGameplayEffectHandle>> ActiveEffectsMap;

	void RevokeAllAppliedEffects();
	void FindNearlyAllies(TArray<APCBaseUnitCharacter*>& OutAllies) const;
};
