// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Item/PCPlayerInventoryWidget.h"

#include "Components/Image.h"

#include "GameFramework/PlayerState/PCPlayerState.h"
#include "Item/PCPlayerInventory.h"
#include "UI/Item/PCItemSlotWidget.h"


void UPCPlayerInventoryWidget::BindToPlayerState(APCPlayerState* NewPlayerState)
{
	if (!NewPlayerState) return;
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

void UPCPlayerInventoryWidget::StartDragItem(int32 SlotIndex)
{
	if (!PlayerInventory) return;
	
	if (PlayerInventory->GetInventory().IsValidIndex(SlotIndex))
	{
		if (ItemSlots[SlotIndex])
		{
			ItemSlots[SlotIndex]->RemoveItem();
		}
	}
}

void UPCPlayerInventoryWidget::EndDragItemAtEmptySpace()
{
	if (!PlayerInventory) return;
	
	if (PlayerInventory->GetInventory().IsValidIndex(DragSlotIndex))
	{
		if (ItemSlots[DragSlotIndex])
		{
			ItemSlots[DragSlotIndex]->SetItem(PlayerInventory->GetInventory()[DragSlotIndex]);	
		}
	}
}

void UPCPlayerInventoryWidget::SetupInventory()
{
	for (int i = 0; i < PlayerInventory->GetInventory().Num(); ++i)
	{
		if (i >= ItemSlots.Num()) break;

		if (ItemSlots[i])
		{
			ItemSlots[i]->SetItem(PlayerInventory->GetInventory()[i]);
		}
	}
}
