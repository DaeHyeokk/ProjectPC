// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayTagContainer.h"
#include "PCItemRecipeRowWidget.generated.h"

class UTextBlock;
class UImage;

/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCItemRecipeRowWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void Setup(FGameplayTag BaseItemTag, FGameplayTag AdvancedItemTag);
	
protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UImage> Img_BaseItemThumbnail;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UImage> Img_AdvancedItemThumbnail;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_AdvancedItemName;
};
