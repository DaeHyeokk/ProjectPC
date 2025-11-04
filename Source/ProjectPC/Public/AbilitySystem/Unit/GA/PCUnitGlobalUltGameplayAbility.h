// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Unit/GA/PCUnitUltAttackGameplayAbility.h"
#include "PCUnitGlobalUltGameplayAbility.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCUnitGlobalUltGameplayAbility : public UPCUnitUltAttackGameplayAbility
{
	GENERATED_BODY()
	
protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;
	
	virtual void OnAttackSucceed(FGameplayEventData Payload) override;

private:
	TArray<TWeakObjectPtr<APCBaseUnitCharacter>> CurrentFieldUnits;
	
};
