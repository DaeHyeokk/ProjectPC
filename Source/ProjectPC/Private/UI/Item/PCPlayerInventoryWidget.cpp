// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Item/PCPlayerInventoryWidget.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/Image.h"

#include "Character/Unit/PCHeroUnitCharacter.h"
#include "GameFramework/PlayerState/PCPlayerState.h"
#include "Item/PCPlayerInventory.h"
#include "UI/Item/PCItemSlotWidget.h"


void UPCPlayerInventoryWidget::BindToPlayerState(APCPlayerState* NewPlayerState, bool IsOwner)
{
	if (!NewPlayerState) return;
	UnBindFromPlayerState();

	bIsOwningInventory = IsOwner;
	
	PlayerInventory = NewPlayerState->GetPlayerInventory();
	PlayerInventory->OnInventoryUpdated.AddUObject(this, &UPCPlayerInventoryWidget::SetupInventory);
	
	for (int32 i = 1; i <= PlayerInventory->MaxInventorySlots; ++i)
	{
		FString SlotName = FString::Printf(TEXT("ItemSlot%d"), i);
		if (UWidget* FoundWidget = GetWidgetFromName(FName(*SlotName)))
		{
			ItemSlots.Add(Cast<UPCItemSlotWidget>(FoundWidget));
		}
	}

	for (int32 i = 0; i < ItemSlots.Num(); ++i)
	{
		if (ItemSlots[i])
		{
			ItemSlots[i]->SetSlotIndex(i);
		}
	}

	SetupInventory();
}

void UPCPlayerInventoryWidget::UnBindFromPlayerState()
{
	if (PlayerInventory)
	{
		PlayerInventory->OnInventoryUpdated.RemoveAll(this);
		PlayerInventory = nullptr;
	}

	for (int32 i = 0; i < ItemSlots.Num(); ++i)
	{
		if (ItemSlots[i])
		{
			ItemSlots[i]->RemoveItem();
		}
	}

	ItemSlots.Empty();

	if (DragImage)
	{
		DragImage->SetVisibility(ESlateVisibility::Collapsed);
		DragImage->SetBrushFromTexture(nullptr);
	}

	DragSlotIndex = -1;
	bIsDragging = false;
	bIsOwningInventory = true;
}

FReply UPCPlayerInventoryWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (!bIsOwningInventory) return FReply::Unhandled();
	Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);

	DragSlotIndex = GetSlotIndexAtMousePos(InGeometry, InMouseEvent.GetScreenSpacePosition());

	if (DragSlotIndex != -1 && PlayerInventory->GetInventory().IsValidIndex(DragSlotIndex))
	{
		if (ItemSlots.IsValidIndex(DragSlotIndex) && ItemSlots[DragSlotIndex] && ItemSlots[DragSlotIndex]->IsItemSet())
		{
			FEventReply Reply = UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton);
			return Reply.NativeReply;
		}
	}

	return FReply::Unhandled();
}

void UPCPlayerInventoryWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent,
	UDragDropOperation*& OutOperation)
{
	if (!bIsOwningInventory) return;
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

	if (DragSlotIndex == -1 || !ItemSlots.IsValidIndex(DragSlotIndex))
		return;

	if (auto DragDropOp = NewObject<UDragDropOperation>())
	{
		DragDropOp->Payload = this;
		DragDropOp->DefaultDragVisual = nullptr;
		DragDropOp->Pivot = EDragPivot::MouseDown;
		
		if (PlayerInventory)
		{
			if (DragImage && ItemSlots[DragSlotIndex])
			{
				if (UTexture2D* Thumbnail = ItemSlots[DragSlotIndex]->GetThumbnail())
				{
					ItemSlots[DragSlotIndex]->RemoveItem();
					DragImage->SetBrushFromTexture(Thumbnail);
					DragImage->SetVisibility(ESlateVisibility::HitTestInvisible);
				}
			}
		}

		bIsDragging = true;
		OutOperation = DragDropOp;
	}
}

bool UPCPlayerInventoryWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	if (!bIsOwningInventory) return false;
	Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);

	if (!InOperation || !PlayerInventory)
		return false;

	auto DropSlotIndex = GetSlotIndexAtMousePos(InGeometry, InDragDropEvent.GetScreenSpacePosition());

	if (DropSlotIndex != -1 && PlayerInventory->GetInventory().IsValidIndex(DropSlotIndex)
		&& DropSlotIndex != DragSlotIndex)
	{
		PlayerInventory->EndDragItem(DragSlotIndex, DropSlotIndex);
	}
	else
	{
		LastMousePos = UWidgetLayoutLibrary::GetMousePositionOnPlatform();
		PlayerInventory->EndDragItem(DragSlotIndex, LastMousePos);
	}

	if (DragImage)
	{
		DragImage->SetVisibility(ESlateVisibility::Collapsed);
		DragImage->SetBrushFromTexture(nullptr);
	}

	if (DropSlotIndex == -1 || DropSlotIndex == DragSlotIndex
		|| !PlayerInventory->GetInventory().IsValidIndex(DropSlotIndex)
		|| (PlayerInventory->GetInventory().IsValidIndex(DragSlotIndex)
			&& PlayerInventory->GetInventory()[DropSlotIndex] == PlayerInventory->GetInventory()[DragSlotIndex]))
	{
		if (ItemSlots.IsValidIndex(DragSlotIndex) && ItemSlots[DragSlotIndex])
		{
			ItemSlots[DragSlotIndex]->SetItem(PlayerInventory->GetInventory()[DragSlotIndex]);
		}
	}

	DragSlotIndex = -1;
	bIsDragging = false;

	return true;
}

void UPCPlayerInventoryWidget::NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	if (!bIsOwningInventory) return;
	Super::NativeOnDragCancelled(InDragDropEvent, InOperation);

	if (!PlayerInventory)
		return;

	LastMousePos = UWidgetLayoutLibrary::GetMousePositionOnPlatform();
	PlayerInventory->EndDragItem(DragSlotIndex, LastMousePos);
	
	if (DragImage)
	{
		DragImage->SetVisibility(ESlateVisibility::Hidden);
		DragImage->SetBrushFromTexture(nullptr);
	}

	if (ItemSlots.IsValidIndex(DragSlotIndex) && ItemSlots[DragSlotIndex])
	{
		ItemSlots[DragSlotIndex]->SetItem(PlayerInventory->GetInventory()[DragSlotIndex]);
	}

	DragSlotIndex = -1;
	bIsDragging = false;
}

void UPCPlayerInventoryWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	
	if (DragImage && bIsDragging)
	{
		const FVector2D MouseAbs = UWidgetLayoutLibrary::GetMousePositionOnPlatform();
		const FVector2D Local = MyGeometry.AbsoluteToLocal(MouseAbs);

		DragImage->SetRenderTranslation(Local - DragImage->GetDesiredSize() * 0.2f);
		// - DragImage->GetDesiredSize() * 0.5f
	}
}

void UPCPlayerInventoryWidget::SetupInventory()
{
	for (int i = 0; i < ItemSlots.Num(); ++i)
	{
		if (ItemSlots[i])
		{
			if (PlayerInventory->GetInventory().IsValidIndex(i))
			{
				ItemSlots[i]->SetItem(PlayerInventory->GetInventory()[i]);
			}
			else
			{
				ItemSlots[i]->RemoveItem();
			}
		}
	}
}

int32 UPCPlayerInventoryWidget::GetSlotIndexAtMousePos(const FGeometry& InGeometry, const FVector2d& MousePos)
{
	for (int32 i = 0; i < ItemSlots.Num(); ++i)
	{
		if (ItemSlots[i])
		{
			FGeometry SlotGeometry = ItemSlots[i]->GetCachedGeometry();
			FVector2D LocalPosition = SlotGeometry.AbsoluteToLocal(MousePos);

			if (LocalPosition.X >= 0 && LocalPosition.X <= SlotGeometry.GetLocalSize().X &&
				LocalPosition.Y >= 0 && LocalPosition.Y <= SlotGeometry.GetLocalSize().Y)
			{
				return i;
			}
		}
	}

	return -1;
}
