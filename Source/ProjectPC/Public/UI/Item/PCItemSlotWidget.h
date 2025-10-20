// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Item/PCItemData.h"
#include "PCItemSlotWidget.generated.h"

class UPCItemInfoWidget;
class UPCItemRecipeWidget;
class UImage;

/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCItemSlotWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual bool Initialize() override;
	
	UPROPERTY(EditDefaultsOnly, Category = "ItemRecipeWidgetClass")
	TSubclassOf<UUserWidget> ItemRecipeWidgetClass;
	UPROPERTY(EditDefaultsOnly, Category = "ItemInfoWidgetClass")
	TSubclassOf<UUserWidget> ItemInfoWidgetClass;
	
	UPROPERTY()
	UPCItemRecipeWidget* ItemRecipeWidget;
	UPROPERTY()
	UPCItemInfoWidget* ItemInfoWidget;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UImage* Img_Item;
	
private:
	int32 SlotIndex = -1;
	bool bIsItemSet = false;

	UPROPERTY()
	UTexture2D* CachedThumbnail;
	
public:
	void SetSlotIndex(int32 NewSlotIndex);
	void SetItem(FGameplayTag NewItemTag);
	void RemoveItem();

	UTexture2D* GetThumbnail() const;
	bool IsItemSet() const;

protected:
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
};
