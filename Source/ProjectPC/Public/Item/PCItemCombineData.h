// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataTable.h"
#include "PCItemCombineData.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct PROJECTPC_API FPCItemCombineData : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Categories = "Item.Type.Base"))
	FGameplayTag ItemTag1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Categories = "Item.Type.Base"))
	FGameplayTag ItemTag2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Categories = "Item.Type.Advanced,Item.Type.Symbol"))
	FGameplayTag ResultItemTag;
};
