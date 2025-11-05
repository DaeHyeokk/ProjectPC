// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "PCDataAsset_UnitDefinitionReg.generated.h"

class UPCDataAsset_UnitDefinition;
/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCDataAsset_UnitDefinitionReg : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TMap<FGameplayTag, TObjectPtr<UPCDataAsset_UnitDefinition>> UnitDefinitionByTagMap;

	UFUNCTION(BlueprintCallable)
	UPCDataAsset_UnitDefinition* FindUnitDefinition(const FGameplayTag UnitTag) const
	{
		if (const TObjectPtr<UPCDataAsset_UnitDefinition>* Found = UnitDefinitionByTagMap.Find(UnitTag))
			return Found->Get();
		return nullptr;
	}

	void GatherAllRegisteredHeroTags(FGameplayTagContainer& OutTags) const;
};
