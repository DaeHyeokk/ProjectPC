// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PCWidgetIconData.generated.h"


USTRUCT(BlueprintType)
struct FStageIconVariant
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Icon")
	UTexture2D* Current = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Icon")
	UTexture2D* UpComing = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Icon")
	UTexture2D* Victory = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Icon")
	UTexture2D* Defeat = nullptr;
	
};
/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCWidgetIconData : public UDataAsset
{
	GENERATED_BODY()
	
public:
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Icons")
	FStageIconVariant PvP;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Icons")
	FStageIconVariant PvE;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Icons")
	FStageIconVariant Carousel;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Icons")
	FStageIconVariant Start;
	
	
};
