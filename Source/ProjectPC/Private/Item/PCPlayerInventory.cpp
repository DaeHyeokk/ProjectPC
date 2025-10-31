// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/PCPlayerInventory.h"

#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"

#include "Character/Unit/PCHeroUnitCharacter.h"
#include "Component/PCUnitEquipmentComponent.h"
#include "GameFramework/GameMode/PCCombatGameMode.h"
#include "GameFramework/GameState/PCCombatGameState.h"
#include "GameFramework/HelpActor/PCPlayerBoard.h"
#include "GameFramework/PlayerState/PCPlayerState.h"
#include "GameFramework/WorldSubsystem/PCItemManagerSubsystem.h"


void UPCPlayerInventory::OnRep_Inventory()
{
	OnInventoryUpdated.Broadcast();
}

bool UPCPlayerInventory::AddItemToInventory(FGameplayTag AddedItemTag)
{
	if (Inventory.Num() < MaxInventorySlots)
	{
		if (const auto ItemManagerSubsystem = GetWorld()->GetSubsystem<UPCItemManagerSubsystem>())
		{
			if (const auto NewItem = ItemManagerSubsystem->GetItemData(AddedItemTag))
			{
				if (NewItem->IsValid())
				{
					Inventory.Add(AddedItemTag);
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
		if (const auto ItemManagerSubsystem = GetWorld()->GetSubsystem<UPCItemManagerSubsystem>())
		{
			auto NewItemTag = ItemManagerSubsystem->CombineItem(Inventory[ItemIndex1], Inventory[ItemIndex2]);
			if (const auto NewItem = ItemManagerSubsystem->GetItemData(NewItemTag))
			{
				if (NewItem->IsValid())
				{
					Inventory[ItemIndex1] = NewItemTag;
					RemoveItemFromInventory(ItemIndex2);
					return;
				}
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

void UPCPlayerInventory::EndDragItem(int32 DraggedInventoryIndex, int32 TargetInventoryIndex)
{
	if (Inventory.IsValidIndex(DraggedInventoryIndex) && Inventory.IsValidIndex(TargetInventoryIndex))
	{
		Server_DropItemAtInventory(DraggedInventoryIndex, TargetInventoryIndex);
	}
}

void UPCPlayerInventory::EndDragItem(int32 DraggedInventoryIndex, const FVector2D& DroppedScreenLoc)
{
	if (!Inventory.IsValidIndex(DraggedInventoryIndex))
		return;

	FHitResult Hit;
	if (auto PS = Cast<APlayerState>(GetOwner()))
	{
		if (auto PC = Cast<APlayerController>(PS->GetPlayerController()))
		{
			PC->GetHitResultAtScreenPosition(DroppedScreenLoc, ECC_Visibility, true, Hit);
		}
	}

	const FVector WorldLoc = Hit.bBlockingHit ? Hit.ImpactPoint : FVector::ZeroVector;
	Server_DropItemAtOutsideInventory(DraggedInventoryIndex, Hit.GetActor(), WorldLoc);
}

void UPCPlayerInventory::Server_DropItemAtInventory_Implementation(int32 DraggedInventoryIndex, int32 TargetInventoryIndex)
{
	if (Inventory.IsValidIndex(DraggedInventoryIndex) && Inventory.IsValidIndex(TargetInventoryIndex))
	{
		CombineItem(TargetInventoryIndex, DraggedInventoryIndex);
	}
}

void UPCPlayerInventory::DropItemAtOutsideInventory(int32 DraggedInventoryIndex, const FVector& DroppedWorldLoc)
{
	if (Inventory.IsValidIndex(DraggedInventoryIndex))
	{
		if (auto PS = Cast<APCPlayerState>(GetOwner()))
		{
			if (auto PB = PS->PlayerBoard)
			{
				bool bIsOnField = false;
				int32 X = -1;
				int32 Y = -1;
				int32 BenchIndex = -1;
				FVector Snap = DroppedWorldLoc;
				
				if (PB->WorldAnyTile(DroppedWorldLoc, false, bIsOnField, Y, X, BenchIndex, Snap))
				{
					// DroppedWorldLoc 위치에 Unit이 존재하면 아이템 장착 시도
					if (APCBaseUnitCharacter* Unit = bIsOnField ? PB->GetFieldUnit(Y, X) : PB->GetBenchUnit(BenchIndex))
					{
						if (UPCUnitEquipmentComponent* EquipmentComp = Unit->GetEquipmentComponent())
						{
							if (EquipmentComp->TryEquipItem(Inventory[DraggedInventoryIndex]))
							{
								RemoveItemFromInventory(DraggedInventoryIndex);
							}
						}
					}
				}
			}
		}
	}
}

void UPCPlayerInventory::Server_DropItemAtOutsideInventory_Implementation(int32 DraggedInventoryIndex, AActor* HitActor, const FVector& DroppedWorldLoc)
{
	if (!Inventory.IsValidIndex(DraggedInventoryIndex))
		return;

	// HitActor가 유닛으로 캐스팅 성공하면 (HitActor가 유닛이면) 아이템 장착 시도
	if (APCBaseUnitCharacter* HitUnit = Cast<APCBaseUnitCharacter>(HitActor))
	{
		if (auto PS = Cast<APCPlayerState>(GetOwner()))
		{
			if (HitUnit->GetTeamIndex() == PS->SeatIndex)
			{
				if (auto Equip = HitUnit->GetEquipmentComponent())
				{
					if (Equip->TryEquipItem(Inventory[DraggedInventoryIndex]))
					{
						RemoveItemFromInventory(DraggedInventoryIndex);
						return;
					}
				}
			}
		}
	}

	if (APCCombatGameState* PCCombatState = GetWorld()->GetGameState<APCCombatGameState>())
	{
		bool bIsBattle = PCCombatState->bIsbattle();
		if (!bIsBattle)
		{
			// 타일 기반으로도 유닛 추적 시도
			DropItemAtOutsideInventory(DraggedInventoryIndex, DroppedWorldLoc);
		}
	}
}

void UPCPlayerInventory::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UPCPlayerInventory, Inventory);
}

