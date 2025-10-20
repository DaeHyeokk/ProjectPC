// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Unit/GA/PCUnitPeriodPulseGameplayAbility.h"
#include "PCUnitManaRegenGameplayAbility.generated.h"

class APCCombatGameState;
/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCUnitManaRegenGameplayAbility : public UPCUnitPeriodPulseGameplayAbility
{
	GENERATED_BODY()
	
public:
	UPCUnitManaRegenGameplayAbility();
	
protected:
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	
	virtual void OnPulseTick() override;

private:
	TWeakObjectPtr<APCCombatGameState> CachedCombatGameState;
	
};
