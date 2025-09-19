// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseGameplayTags.h"
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

	FGameplayTag PhysicalDamageTypeTag = UnitGameplayTags::Unit_DamageType_Physical;
	FGameplayTag MagicDamageTypeTag = UnitGameplayTags::Unit_DamageType_Magic;

	FGameplayTag DamageCallerTag = GameplayEffectTags::GE_Caller_Damage;
	FGameplayTag HealCallerTag = GameplayEffectTags::GE_Caller_Stat_CurrentHealth;
	FGameplayTag HealthChangeGEKeyTag = GameplayEffectTags::GE_Class_HealthChange_Instant;
	
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
