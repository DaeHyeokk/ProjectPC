// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "PCDataAsset_UnitAbilityConfig.generated.h"

class UGameplayEffect;

USTRUCT(BlueprintType)
struct FAbilityConfig
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category="Cost")
	TSoftClassPtr<UGameplayEffect> CostGEClass;
	
	UPROPERTY(EditDefaultsOnly, Category="Cost")
	FGameplayTag CostCallerTag;

	UPROPERTY(EditDefaultsOnly, Category="Cooldown")
	TSoftClassPtr<UGameplayEffect> CooldownGEClass;
	
	UPROPERTY(EditDefaultsOnly, Category="Cooldown")
	FGameplayTag CooldownCallerTag;
};

/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCDataAsset_UnitAbilityConfig : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Abilities")
	TMap<FGameplayTag, FAbilityConfig> AbilityConfigMap;
	
	
};
