// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "UPCUnitDamageExec.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCUnitDamageExec : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()
	
public:
	UPCUnitDamageExec();

	// static FGameplayTag PhysicalDamageTypeTag;
	// static FGameplayTag MagicDamageTypeTag;
	// static FGameplayTag DamageCallerTag;
	// static FGameplayTag HealCallerTag;
	// static FGameplayTag HealGEKeyTag;
	
protected:
	struct FCaptureDefs
	{
		// Target Attribute
		FGameplayEffectAttributeCaptureDefinition PhysicalDefense;
		FGameplayEffectAttributeCaptureDefinition MagicDefense;
		
		// Source Attribute
		FGameplayEffectAttributeCaptureDefinition CritChance;
		FGameplayEffectAttributeCaptureDefinition CritMultiplier;
		FGameplayEffectAttributeCaptureDefinition PhysDamageMultiplier;
		FGameplayEffectAttributeCaptureDefinition MagicDamageMultiplier;
		FGameplayEffectAttributeCaptureDefinition LifeSteal;
		FGameplayEffectAttributeCaptureDefinition SpellVamp;
		
	} Captures;

	virtual void Execute_Implementation(
		const FGameplayEffectCustomExecutionParameters& ExecutionParams,
		FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;

	const UGameplayEffect* ResolveHealGE(const UWorld* World) const;
};
