// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "GameplayTagContainer.h"
#include "AbilitySystem/Unit/EffectSpec/PCEffectCoreTypes.h"
#include "PCAbilityConfigTypes.generated.h"

USTRUCT(BlueprintType)
struct FAbilityConfig
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category="Cost", meta=(InlineEditConditionToggle))
	bool bUseCost = false;
	
	UPROPERTY(EditDefaultsOnly, Category="Cost", meta=(EditCondition="bUseCost", Categories="GE.Class"))
	FGameplayTag CostEffectClassTag;

	UPROPERTY(EditDefaultsOnly, Category="Cost", meta=(EditCondition="bUseCost"))
	FGameplayAttribute CostGameplayAttribute;
	
	UPROPERTY(EditDefaultsOnly, Category="Cost", meta=(EditCondition="bUseCost", Categories="GE.Caller"))
	FGameplayTag CostCallerTag;
	
	UPROPERTY(EditDefaultsOnly, Category="Coooldown", meta=(InlineEditConditionToggle))
	bool bUseCooldown = false;
	
	UPROPERTY(EditDefaultsOnly, Category="Cooldown", meta=(EditCondition="bUseCooldown", Categories="GE.Class"))
	FGameplayTag CooldownEffectClassTag;
	
	UPROPERTY(EditDefaultsOnly, Category="Cooldown", meta=(EditCondition="bUseCooldown", Categories="GE.Caller"))
	FGameplayTag CooldownCallerTag;
	
	UPROPERTY(EditDefaultsOnly, Category="Projectile")
	bool bSpawnProjectile = false;
	
	UPROPERTY(EditDefaultsOnly, Category="Projectile", meta=(EditCondition="bSpawnProjectile"))
	FPCEffectSpecList ProjectilePayloadEffectSpecs;
	
	// 어빌리티 발동 즉시 적용하는 GE 스펙
	UPROPERTY(EditDefaultsOnly, Category="Effect|Immediate")
	FPCEffectSpecList OnActivatedEffectSpecs;

	// 어빌리티 커밋 시 적용하는 GE 스펙
	UPROPERTY(EditDefaultsOnly, Category="Effect|Immediate")
	FPCEffectSpecList OnCommittedEffectSpecs;
	
	// 어벌리티 발동 중 받는 이벤트에 따라 적용하는 GE 스펙
	UPROPERTY(EditDefaultsOnly, Category="Effect|Situation")
	TMap<FGameplayTag, FPCEffectSpecList> OnReceivedEventEffectsMap;
};