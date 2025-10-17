// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Unit/GA/PCUnitWaitEventGameplayAbility.h"
#include "PCSynergyBladeMasterGameplayAbility.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCSynergyBladeMasterGameplayAbility : public UPCUnitWaitEventGameplayAbility
{
	GENERATED_BODY()
	
public:
	UPCSynergyBladeMasterGameplayAbility();

protected:
	virtual TArray<FActiveGameplayEffectHandle> ApplyCommittedEffectSpec(UAbilitySystemComponent* ASC, const AActor* Target = nullptr) override;
	
	UPROPERTY(EditDefaultsOnly, Category="BladeMaster|Config")
	FScalableFloat BonusAttackCount;
};
