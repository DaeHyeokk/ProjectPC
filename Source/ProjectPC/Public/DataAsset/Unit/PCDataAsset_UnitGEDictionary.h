// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "PCDataAsset_UnitGEDictionary.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCDataAsset_UnitGEDictionary : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, Category="GAS|GE")
	TMap<FGameplayTag, TSoftClassPtr<UGameplayEffect>> Entries;
};
