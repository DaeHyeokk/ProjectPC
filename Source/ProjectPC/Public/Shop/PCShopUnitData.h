// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
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
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName UnitName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 UnitCost;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName UnitClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName UnitOrigin;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* UnitTexture;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 UnitCount;
};
