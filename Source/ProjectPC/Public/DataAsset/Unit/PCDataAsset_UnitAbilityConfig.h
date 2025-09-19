// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "GameplayTagContainer.h"
#include "AbilitySystem/Unit/EffectSpec/PCEffectSpec.h"
#include "Engine/DataAsset.h"
#include "PCDataAsset_UnitAbilityConfig.generated.h"

class UGameplayEffect;

USTRUCT(BlueprintType)
struct FPCEffectSpecList
{
	GENERATED_BODY()

	// Instanced는 여기에 달려 있어야 인라인 생성/저장이 됨
	UPROPERTY(EditDefaultsOnly, Instanced)
	TArray<TObjectPtr<UPCEffectSpec>> EffectSpecs;
};

USTRUCT(BlueprintType)
struct FAbilityConfig
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category="Cost", meta=(InlineEditConditionToggle))
	bool bUseCost = false;
	
	UPROPERTY(EditDefaultsOnly, Category="Cost", meta=(EditCondition="bUseCost"))
	FGameplayTag CostEffectClassTag;

	UPROPERTY(EditDefaultsOnly, Category="Cost", meta=(EditCondition="bUseCost"))
	FGameplayAttribute CostGameplayAttribute;
	
	UPROPERTY(EditDefaultsOnly, Category="Cost", meta=(EditCondition="bUseCost"))
	FGameplayTag CostCallerTag;
	
	UPROPERTY(EditDefaultsOnly, Category="Coooldown", meta=(InlineEditConditionToggle))
	bool bUseCooldown = false;
	
	UPROPERTY(EditDefaultsOnly, Category="Cooldown", meta=(EditCondition="bUseCooldown"))
	FGameplayTag CooldownEffectClassTag;
	
	UPROPERTY(EditDefaultsOnly, Category="Cooldown", meta=(EditCondition="bUseCooldown"))
	FGameplayTag CooldownCallerTag;
	
	UPROPERTY(EditDefaultsOnly, Category="Spawn")
	bool bSpawnProjectile = false;
	
	// 어빌리티 발동 즉시 적용하는 GE 스펙
	UPROPERTY(EditDefaultsOnly, Category="Effect|Immediate")
	FPCEffectSpecList OnActivatedEffectSpecs;
	
	UPROPERTY(EditDefaultsOnly, Category="Effect|Notify")
	TMap<FGameplayTag, FPCEffectSpecList> OnReceivedEventEffectsMap;
};

/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCDataAsset_UnitAbilityConfig : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	bool TryFindAbilityConfigByTag(const FGameplayTag& AbilityTag, FAbilityConfig& OutConfig) const;
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Abilities")
	TMap<FGameplayTag, FAbilityConfig> AbilityConfigMap;
};
