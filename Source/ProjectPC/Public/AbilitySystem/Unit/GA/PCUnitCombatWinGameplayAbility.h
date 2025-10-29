// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PCBaseUnitGameplayAbility.h"
#include "PCUnitCombatWinGameplayAbility.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCUnitCombatWinGameplayAbility : public UPCBaseUnitGameplayAbility
{
	GENERATED_BODY()
	
public:
	UPCUnitCombatWinGameplayAbility();
	
protected:
	virtual void ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData) override;
};
