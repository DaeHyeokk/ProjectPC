// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PCItemData.h"
#include "PCPlayerInventory.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnInventoryUpdated);

class APCHeroUnitCharacter;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTPC_API UPCPlayerInventory : public UActorComponent
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	int32 MaxInventorySlots = 10;

	FOnInventoryUpdated OnInventoryUpdated;
	
private:
	UPROPERTY(ReplicatedUsing = OnRep_Inventory)
	TArray<FGameplayTag> Inventory;
	
	UFUNCTION()
	void OnRep_Inventory();

public:
	const TArray<FGameplayTag>& GetInventory() const { return Inventory; }
	int32 GetInventorySize() const { return Inventory.Num(); }

	bool AddItemToInventory(FGameplayTag AddedItemTag);
	void RemoveItemFromInventory(int32 ItemIndex);

	void CombineItem(int32 ItemIndex1, int32 ItemIndex2);
	void SwapItem(int32 ItemIndex1, int32 ItemIndex2);
	
	void EndDragItem(int32 DraggedInventoryIndex, int32 TargetInventoryIndex);
	void EndDragItem(int32 DraggedInventoryIndex, const FVector2D& DroppedScreenLoc);

	UFUNCTION(Server, Reliable)
	void DropItemAtInventory(int32 DraggedInventoryIndex, int32 TargetInventoryIndex);
	UFUNCTION(Server, Reliable)
	void DropItemAtOutsideInventory(int32 DraggedInventoryIndex, const FVector& DroppedWorldLoc);

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
