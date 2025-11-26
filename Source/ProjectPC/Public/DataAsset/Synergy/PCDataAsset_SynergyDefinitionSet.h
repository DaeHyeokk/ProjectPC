// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PCDataAsset_SynergyDefinitionSet.generated.h"

class UPCDataAsset_SynergyData;
class UPCSynergyBase;

USTRUCT()
struct FSynergyDefinition
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<UPCSynergyBase> SynergyClass;
	
	UPROPERTY(EditAnywhere)
	TObjectPtr<UPCDataAsset_SynergyData> SynergyData;
};
/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCDataAsset_SynergyDefinitionSet : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere)
	TArray<FSynergyDefinition> Definitions;
	
	
};
