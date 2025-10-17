// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Unit/GA/PCUnitWaitEventGameplayAbility.h"
#include "PCSynergyMechanicGameplayAbility.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCSynergyMechanicGameplayAbility : public UPCUnitWaitEventGameplayAbility
{
	GENERATED_BODY()

public:
	UPCSynergyMechanicGameplayAbility();

protected:
	virtual TArray<FActiveGameplayEffectHandle> ApplyCommittedEffectSpec(UAbilitySystemComponent* ASC, const AActor* Target = nullptr) override;
	virtual bool ShouldCommitOnEvent_Implementation(const FGameplayEventData& Payload) override;
	
	UPROPERTY(EditDefaultsOnly, Category="Mechanic|Config")
	FScalableFloat ExecutionHpThresholdPercent;
;
};
