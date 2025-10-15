// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PCItemData.h"
#include "PCPlayerInventory.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnInventoryUpdated);

UENUM(BlueprintType)
enum class EItemDropTarget : uint8
{
	None UMETA(DisplayName="None"),
	Inventory UMETA(DisplayName="Inventory"),
	Unit UMETA(DisplayName="Unit")
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTPC_API UPCPlayerInventory : public UActorComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	int32 MaxInventorySlots = 10;

	int32 CachedDragItemIndex = -1;

	FOnInventoryUpdated OnInventoryUpdated;
	
private:
	UPROPERTY(ReplicatedUsing = OnRep_Inventory)
	TArray<FPCItemData> Inventory;

public:
	const TArray<FPCItemData>& GetInventory() const { return Inventory; }
	
	bool AddItemToInventory(FGameplayTag AddedItemTag);
	void RemoveItemFromInventory(int32 ItemIndex);
	
	void CombineItem(int32 ItemIndex1, int32 ItemIndex2);
	void SwapItem(int32 ItemIndex1, int32 ItemIndex2);

	void StartDragItem(int32 ItemIndex);
	void EndDragItem(EItemDropTarget DropTarget, int32 TargetInventoryIndex);

private:
	UFUNCTION()
	void OnRep_Inventory();

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
