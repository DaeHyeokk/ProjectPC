// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Item/PCPlayerInventory.h"
#include "PCPlayerInventoryWidget.generated.h"

class UCanvasPanel;
class UImage;
class APCPlayerState;
class UPCItemSlotWidget;

/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCPlayerInventoryWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY()
	TArray<UPCItemSlotWidget*> ItemSlots;

public:
	void BindToPlayerState(APCPlayerState* NewPlayerState);

private:
	UPROPERTY()
	UPCPlayerInventory* PlayerInventory;
	
	int32 DragSlotIndex = -1;
	
public:
	void StartDragItem(int32 SlotIndex);
	void EndDragItemAtEmptySpace();

private:
	void SetupInventory();
};
