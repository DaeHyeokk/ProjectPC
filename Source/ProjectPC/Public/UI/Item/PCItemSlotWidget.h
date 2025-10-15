// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Item/PCItemData.h"
#include "PCItemSlotWidget.generated.h"

class UImage;

/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCItemSlotWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UImage* Img_Item;
	
private:
	int32 SlotIndex;
	bool bIsItemSet = false;

	UPROPERTY()
	UTexture2D* CachedThumbnail;
	
public:
	void SetSlotIndex(int32 NewSlotIndex);
	void SetItem(const FPCItemData& NewItem);
	void RemoveItem();

	UTexture2D* GetThumbnail() const;
	bool IsItemSet() const;

	bool IsUnderLocation(const FVector2D& ScreenPos) const;
};
