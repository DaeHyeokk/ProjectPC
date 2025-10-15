// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/PCPlayerInventory.h"

#include "Net/UnrealNetwork.h"

#include "GameFramework/WorldSubsystem/PCItemManagerSubsystem.h"


bool UPCPlayerInventory::AddItemToInventory(FGameplayTag AddedItemTag)
{
	if (Inventory.Num() < MaxInventorySlots)
	{
		if (auto ItemManagerSubsystem = GetWorld()->GetSubsystem<UPCItemManagerSubsystem>())
		{
			if (const auto NewItem = ItemManagerSubsystem->GetItemData(AddedItemTag))
			{
				if (NewItem->IsValid())
				{
					Inventory.Add(*NewItem);
					return true;
				}
			}
		}
	}

	return false;
}

void UPCPlayerInventory::RemoveItemFromInventory(int32 ItemIndex)
{
	if (!Inventory.IsValidIndex(ItemIndex))
		return;

	Inventory.RemoveAt(ItemIndex);
}

void UPCPlayerInventory::CombineItem(int32 ItemIndex1, int32 ItemIndex2)
{
	if (!Inventory.IsValidIndex(ItemIndex1) || !Inventory.IsValidIndex(ItemIndex2))
		return;

	if (Inventory[ItemIndex1].IsValid() && Inventory[ItemIndex2].IsValid())
	{
		if (auto ItemManagerSubsystem = GetWorld()->GetSubsystem<UPCItemManagerSubsystem>())
		{
			if (const auto NewItem = ItemManagerSubsystem->CombineItem(Inventory[ItemIndex1].ItemTag, Inventory[ItemIndex2].ItemTag))
			{
				Inventory[ItemIndex1] = *NewItem;
				RemoveItemFromInventory(ItemIndex2);
				return;
			}
		}

		SwapItem(ItemIndex1, ItemIndex2);
	}
}

void UPCPlayerInventory::SwapItem(int32 ItemIndex1, int32 ItemIndex2)
{
	if (!Inventory.IsValidIndex(ItemIndex1) || !Inventory.IsValidIndex(ItemIndex2))
		return;
	
	Inventory.Swap(ItemIndex1, ItemIndex2);
}

void UPCPlayerInventory::StartDragItem(int32 ItemIndex)
{
	if (Inventory.IsValidIndex(ItemIndex))
	{
		CachedDragItemIndex = ItemIndex;
	}
}

void UPCPlayerInventory::EndDragItem(EItemDropTarget DropTarget, int32 TargetInventoryIndex)
{
	switch (DropTarget)
	{
	case EItemDropTarget::Inventory:
		{
			if (Inventory.IsValidIndex(CachedDragItemIndex) && Inventory.IsValidIndex(TargetInventoryIndex))
			{
				CombineItem(TargetInventoryIndex, CachedDragItemIndex);
			}
			
			break;
		}
		
	case EItemDropTarget::Unit:
		{
			if (Inventory.IsValidIndex(CachedDragItemIndex))
			{
				RemoveItemFromInventory(CachedDragItemIndex);
			}

			break;
		}
		
	default:
		break;
	}

	CachedDragItemIndex = -1;
}

void UPCPlayerInventory::OnRep_Inventory()
{
	OnInventoryUpdated.Broadcast();
}

void UPCPlayerInventory::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UPCPlayerInventory, Inventory);
}

