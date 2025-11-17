// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseGameplayTags.h"
#include "GameplayEffectExecutionCalculation.h"
#include "PCUnitDamageExec.generated.h"

UENUM()
enum EDamageType
{
	Physical,
	Magic,
	TrueDamage
};

/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCUnitDamageExec : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()
	
public:
	UPCUnitDamageExec();

protected:
	FGameplayTag DamageCallerTag = GameplayEffectTags::GE_Caller_Damage;
	FGameplayTag HealCallerTag = GameplayEffectTags::GE_Caller_Heal;
	FGameplayTag HealGEClassTag = GameplayEffectTags::GE_Class_Unit_Heal;

	FGameplayTag BasicHitSucceedEventTag = UnitGameplayTags::Unit_Event_HitSucceed_Basic;
	FGameplayTag UltimateHitSucceedEventTag = UnitGameplayTags::Unit_Event_HitSucceed_Ultimate;
	FGameplayTag BonusDmgHitSucceedEventTag = UnitGameplayTags::Unit_Event_HitSucceed_BonusDamage;
	FGameplayTag OnHitEventTag = UnitGameplayTags::Unit_Event_OnHit;
	
	struct FCaptureDefs
	{
		// Target Attribute
		FGameplayEffectAttributeCaptureDefinition PhysicalDefense;
		FGameplayEffectAttributeCaptureDefinition MagicDefense;
		FGameplayEffectAttributeCaptureDefinition Evasion;

		// Target Attribute - Synergy
		FGameplayEffectAttributeCaptureDefinition FlatDamageBlock;
		FGameplayEffectAttributeCaptureDefinition IncomingPhysicalDamageMultiplier; // 최종으로 받는 물리데미지 배율
		FGameplayEffectAttributeCaptureDefinition IncomingMagicDamageMultiplier; // 최종으로 받는 마법데미지 배율
		
		// Source Attribute
		FGameplayEffectAttributeCaptureDefinition CritChance;
		FGameplayEffectAttributeCaptureDefinition CritMultiplier;
		FGameplayEffectAttributeCaptureDefinition PhysDamageMultiplier;
		FGameplayEffectAttributeCaptureDefinition MagicDamageMultiplier;
		FGameplayEffectAttributeCaptureDefinition DamageMultiplier;
		FGameplayEffectAttributeCaptureDefinition LifeSteal;
	} Captures;

	virtual void Execute_Implementation(
		const FGameplayEffectCustomExecutionParameters& ExecutionParams,
		FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;

	const UGameplayEffect* ResolveHealGE(const UWorld* World) const;
};
