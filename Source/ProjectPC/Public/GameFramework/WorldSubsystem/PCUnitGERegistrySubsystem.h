// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DataAsset/Unit/PCDataAsset_UnitGEDictionary.h"
#include "Subsystems/WorldSubsystem.h"
#include "PCUnitGERegistrySubsystem.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCUnitGERegistrySubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TObjectPtr<UPCDataAsset_UnitGEDictionary> UnitGEDict = nullptr;

	TMap<FGameplayTag, TWeakObjectPtr<UClass>> CachedGEClasses;

	void InitializeUnitGERegistry(UPCDataAsset_UnitGEDictionary* InUnitGEDict, const FGameplayTagContainer& PreloadGEClassKeys);
	UClass* GetGEClass(const FGameplayTag& GEClassTag);
	UGameplayEffect* GetGE_CDO(const FGameplayTag& GEClassTag);

	UGameplayEffect* GetGrantUnitTypeGE_CDO(const FGameplayTag& UnitTypeTag);
};
