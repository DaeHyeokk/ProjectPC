// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "PCLevelMaxXpData.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct PROJECTPC_API FPCLevelMaxXPData : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 PlayerLevel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 MaxXP;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalXP;
};
