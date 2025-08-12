// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "PCShopUnitProbabilityData.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct PROJECTPC_API FPCShopUnitProbabilityData : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 PlayerLevel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Cost1Probability;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Cost2Probability;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Cost3Probability;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Cost4Probability;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Cost5Probability;
};
