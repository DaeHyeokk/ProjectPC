// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Blueprint/UserWidget.h"
#include "PCItemInfoWidget.generated.h"

class UPCItemStatWidget;
class UHorizontalBox;
class UTextBlock;
class UImage;

/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCItemInfoWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void Setup(FGameplayTag ItemTag);
	
protected:
	UPROPERTY(EditDefaultsOnly, Category = "ItemStatWidgetClass")
	TSubclassOf<UUserWidget> ItemStatWidgetClass;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UImage> Img_ItemThumbnail;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_ItemName;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_ItemUniqueEffect;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UHorizontalBox> ItemStatBox;

private:
	FGameplayTag CachedItemTag;

	void SetItemInfo();
};
