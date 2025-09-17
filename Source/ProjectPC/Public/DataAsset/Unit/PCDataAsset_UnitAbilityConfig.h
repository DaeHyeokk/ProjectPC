// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "GameplayTagContainer.h"
#include "AbilitySystem/Unit/EffectSpec/PCEffectSpec.h"
#include "Engine/DataAsset.h"
#include "PCDataAsset_UnitAbilityConfig.generated.h"

class UGameplayEffect;

UENUM(BlueprintType)
enum class EAbilityType : uint8
{
	Attack, 
	Movement,
};

USTRUCT(BlueprintType)
struct FAbilityConfig
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category="Cost", meta=(InlineEditConditionToggle))
	bool bUseCost = false;
	
	UPROPERTY(EditDefaultsOnly, Category="Cost", meta=(EditCondition="bUseCost"))
	FGameplayTag CostGETag;

	UPROPERTY(EditDefaultsOnly, Category="Cost", meta=(EditCondition="bUseCost"))
	FGameplayAttribute CostGameplayAttribute;
	
	UPROPERTY(EditDefaultsOnly, Category="Cost", meta=(EditCondition="bUseCost"))
	FGameplayTag CostCallerTag;
	
	UPROPERTY(EditDefaultsOnly, Category="Coooldown", meta=(InlineEditConditionToggle))
	bool bUseCooldown = false;
	
	UPROPERTY(EditDefaultsOnly, Category="Cooldown", meta=(EditCondition="bUseCooldown"))
	FGameplayTag CooldownGETag;
	
	UPROPERTY(EditDefaultsOnly, Category="Cooldown", meta=(EditCondition="bUseCooldown"))
	FGameplayTag CooldownCallerTag;

	// 어빌리티 발동 즉시 적용하는 GE 스펙
	UPROPERTY(EditDefaultsOnly, Instanced, Category="Effect|Immediate")
	TArray<TObjectPtr<UPCEffectSpec>> OnActivateEffectSpecs;

	//UPROPERTY(EditDefaultsOnly, Instanced, Category="Effect|Notify")
	//TMap<FGameplayTag, TArray<UPCEffectSpec>> OnNotifyEffectsMap;

	UPROPERTY(EditDefaultsOnly, Instanced, Category="Effect|OnHit")
	TArray<TObjectPtr<UPCEffectSpec>> OnHitEffectSpecs;

	//UPROPERTY(EditDefaultsOnly,Instanced, Category="Projectile")
	//TArray
	
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
