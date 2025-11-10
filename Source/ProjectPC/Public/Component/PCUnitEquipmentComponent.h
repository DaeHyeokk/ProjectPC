// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActiveGameplayEffectHandle.h"
#include "GameplayTagContainer.h"
#include "AbilitySystem/Unit/EffectSpec/PCEffectCoreTypes.h"
#include "Components/ActorComponent.h"
#include "PCUnitEquipmentComponent.generated.h"


class APCBaseUnitCharacter;
class UPCUnitEquipmentComponent;
class UPCPlayerInventory;
class UPCItemManagerSubsystem;
class UAbilitySystemComponent;

DECLARE_MULTICAST_DELEGATE(FOnEquipItemChanged);

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
	bool TryEquipItem(const FGameplayTag& ItemTag, bool bIsUnion = false);
	void UnionEquipmentComponent(UPCUnitEquipmentComponent* InEquipmentComp);
	void ReturnAllItemToPlayerInventory(const bool bIsDestroyedHero = false);
	void ReturnItemToPlayerInventory(const FGameplayTag& ItemTag) const;
	
	FORCEINLINE const TArray<FGameplayTag>& GetSlotItemTags() const { return SlotItemTags; }

	FOnEquipItemChanged OnEquipItemChanged;
	
private:
	UPROPERTY(Transient)
	TWeakObjectPtr<APCBaseUnitCharacter> Owner = nullptr;

	UPROPERTY(Transient)
	TWeakObjectPtr<UPCPlayerInventory> OwnerPlayerInventory = nullptr;
	
	UPROPERTY(Transient)
	TWeakObjectPtr<UPCItemManagerSubsystem> ItemManagerSubsystem = nullptr;
	
	TArray<TArray<FActiveGameplayEffectHandle>> SlotActiveEffects;

	int32 MaxSlotSize = 3;
	
	UPROPERTY(VisibleAnywhere, ReplicatedUsing=OnRep_SlotItemTags)
	TArray<FGameplayTag> SlotItemTags;
	
	UFUNCTION()
	void OnRep_SlotItemTags() const;

	void SetItemToSlot(const FGameplayTag& ItemTag, const int32 SlotIndex, const bool bPlayParticle);
	void RemoveItemSlot(const int32 SlotIndex);
	void ApplyItemEffects(const FGameplayTag& ItemTag, const int32 SlotIndex);
	void RemoveSlotActiveEffects(const int32 SlotIndex);
	const FPCEffectSpecList* ResolveItemEffectSpecList(const FGameplayTag& ItemTag) const;
	
	bool HasAuthority() const;
};
