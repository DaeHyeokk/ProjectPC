// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActiveGameplayEffectHandle.h"
#include "GameplayTagContainer.h"
#include "AbilitySystem/Unit/EffectSpec/PCEffectCoreTypes.h"
#include "Components/ActorComponent.h"
#include "PCUnitEquipmentComponent.generated.h"


class UPCPlayerInventory;
class UPCItemManagerSubsystem;
class UAbilitySystemComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTPC_API UPCUnitEquipmentComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UPCUnitEquipmentComponent();

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
public:
	void SetMaxSlotSize(int32 MaxSize) { MaxSlotSize = MaxSize; }
	bool TryEquipItem(const FGameplayTag& ItemTag);
	void UnionEquipmentComponent(UPCUnitEquipmentComponent* InEquipmentComp);
	void ReturnAllItemToPlayerInventory(const bool bIsDestroyedHero = false);
	
	FORCEINLINE const TArray<FGameplayTag>& GetSlotItemTags() const { return SlotItemTags; }
	
private:
	UPROPERTY(Transient)
	TObjectPtr<UAbilitySystemComponent> OwnerASC = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UPCPlayerInventory> OwnerPlayerInventory = nullptr;
	
	UPROPERTY(Transient)
	TObjectPtr<UPCItemManagerSubsystem> ItemManagerSubsystem;
	
	TArray<TArray<FActiveGameplayEffectHandle>> SlotActiveEffects;

	int32 MaxSlotSize = 3;
	
	UPROPERTY(VisibleAnywhere, ReplicatedUsing=OnRep_SlotItemTags)
	TArray<FGameplayTag> SlotItemTags;
	
	UFUNCTION()
	void OnRep_SlotItemTags();

	void SetItemToSlot(const FGameplayTag& ItemTag, const int32 SlotIndex);
	void RemoveItemSlot(const int32 SlotIndex);
	void ReturnItemToPlayerInventory(const FGameplayTag& ItemTag) const;
	void ApplyItemEffects(const FGameplayTag& ItemTag, const int32 SlotIndex);
	void RemoveSlotActiveEffects(const int32 SlotIndex);
	const FPCEffectSpecList* ResolveItemEffectSpecList(const FGameplayTag& ItemTag) const;

	bool HasAuthority() const;
};
