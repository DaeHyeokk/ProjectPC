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

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
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
	void EmptyInventory();

	void CombineItem(int32 ItemIndex1, int32 ItemIndex2);
	void SwapItem(int32 ItemIndex1, int32 ItemIndex2);

	// 인벤토리 아이템 슬롯에 드랍됐을 때
	void EndDragItem(int32 DraggedInventoryIndex, int32 TargetInventoryIndex);
	// 인벤토리 아이템 슬롯이 아닌 곳에 드랍됐을 때
	void EndDragItem(int32 DraggedInventoryIndex, const FVector2D& DroppedScreenLoc);
	
	UFUNCTION(Server, Reliable)
	void Server_DropItemAtInventory(int32 DraggedInventoryIndex, int32 TargetInventoryIndex);

	// 액터 기반 유닛 추적
	UFUNCTION(Server,Reliable)
	void Server_DropItemAtOutsideInventory(int32 DraggedInventoryIndex, AActor* HitActor, const FVector& DroppedWorldLoc);
	
	// 타일 기반 유닛 추적
	void DropItemAtOutsideInventory(int32 DraggedInventoryIndex, const FVector& DroppedWorldLoc);
};
