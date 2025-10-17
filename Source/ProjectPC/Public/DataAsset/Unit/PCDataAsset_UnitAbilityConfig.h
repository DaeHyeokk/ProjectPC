// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "AbilitySystem/Unit/GA/PCAbilityConfigTypes.h"
#include "Engine/DataAsset.h"
#include "PCDataAsset_UnitAbilityConfig.generated.h"


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
