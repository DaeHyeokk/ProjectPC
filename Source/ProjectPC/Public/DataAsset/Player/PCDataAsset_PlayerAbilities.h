// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "PCDataAsset_PlayerAbilities.generated.h"

class UGameplayEffect;
class UGameplayAbility;

/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCDataAsset_PlayerAbilities : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category = "GE")
	TSubclassOf<UGameplayEffect> InitializedEffect;

	UPROPERTY(EditDefaultsOnly, Category = "GE")
	TMap<FGameplayTag, float> InitializedEffectCallerValues;

	UPROPERTY(EditDefaultsOnly, Category = "GE")
	TMap<FGameplayTag, TSubclassOf<UGameplayEffect>> PlayerEffects;

	UPROPERTY(EditDefaultsOnly, Category = "GA")
	TArray<TSubclassOf<UGameplayAbility>> InitializedAbilities;
};
