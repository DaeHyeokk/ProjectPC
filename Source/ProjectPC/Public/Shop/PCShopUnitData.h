// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataTable.h"
#include "PCShopUnitData.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct PROJECTPC_API FPCShopUnitData : public FTableRowBase
{
	GENERATED_BODY()
	
public:
	FPCShopUnitData()
	{
		UnitName = NAME_None;
		UnitCost = 1;
		UnitCount = 22;
		UnitTag = FGameplayTag();
		UnitSpeciesTag = FGameplayTag();
		UnitJobTag = FGameplayTag();
		UnitTexture = nullptr;
	}
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName UnitName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 UnitCost = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 UnitCount = 22;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Categories = "Unit.Type.Hero"))
	FGameplayTag UnitTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Categories = "Synergy.Species"))
	FGameplayTag UnitSpeciesTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Categories = "Synergy.job"))
	FGameplayTag UnitJobTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> UnitTexture;

	bool operator==(const FPCShopUnitData& Other) const
	{
		return UnitName == Other.UnitName && UnitTag == Other.UnitTag;
	}
};
