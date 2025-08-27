// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "PCShopUnitSellingPriceData.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct PROJECTPC_API FPCShopUnitSellingPriceData : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 UnitCost;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 UnitLevel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 UnitSellingPrice;
};
