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
	int32 PlayerLevel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Probability_Cost1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Probability_Cost2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Probability_Cost3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Probability_Cost4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Probability_Cost5;
};
