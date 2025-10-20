// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Blueprint/UserWidget.h"
#include "DataAsset/Unit/PCDataAsset_UnitStatIcon.h"
#include "PCItemStatWidget.generated.h"

class UTextBlock;
class UImage;

/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCItemStatWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void Setup(FGameplayTag StatTag, float StatValue, bool bIsMultiplier);
	
protected:
	UPROPERTY(EditDefaultsOnly, Category = "StatIcon")
	UPCDataAsset_UnitStatIcon* DA_StatIcons;
	
	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	UImage* Img_Stat;

	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	UTextBlock* Text_StatValue;
};
