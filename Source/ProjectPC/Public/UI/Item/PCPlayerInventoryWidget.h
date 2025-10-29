// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Item/PCPlayerInventory.h"
#include "PCPlayerInventoryWidget.generated.h"

class APCHeroUnitCharacter;
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
	TArray<TObjectPtr<UPCItemSlotWidget>> ItemSlots;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UImage> DragImage;

public:
	void BindToPlayerState(APCPlayerState* NewPlayerState);

private:
	UPROPERTY()
	UPCPlayerInventory* PlayerInventory;
	
	int32 DragSlotIndex = -1;
	bool bIsDragging = false;

	FVector2D LastMousePos;

protected:
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual void NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	void SetupInventory();

	int32 GetSlotIndexAtMousePos(const FGeometry& InGeometry, const FVector2d& MousePos);
};
