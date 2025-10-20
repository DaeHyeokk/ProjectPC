// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "DataAsset/Unit/PCDataAsset_UnitAbilityConfig.h"
#include "Engine/DataTable.h"
#include "PCItemData.generated.h"

class UPCDataAsset_ItemEffectData;

/**
 * 
 */
USTRUCT(BlueprintType)
struct PROJECTPC_API FPCItemData : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UPCDataAsset_ItemEffectData> EffectSpecList;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag ItemTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> ItemTexture;

	bool IsValid() const
	{
		return ItemTag.IsValid() && !ItemName.IsNone();
	}
};
