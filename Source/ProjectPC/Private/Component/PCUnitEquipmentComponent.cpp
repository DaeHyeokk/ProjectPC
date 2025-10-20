// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/PCUnitEquipmentComponent.h"

#include "AbilitySystemGlobals.h"
#include "AbilitySystem/Unit/EffectSpec/PCEffectSpec.h"
#include "Net/UnrealNetwork.h"

UPCUnitEquipmentComponent::UPCUnitEquipmentComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);

	//SlotItemTags.SetNum(MaxSlotSize);
}

void UPCUnitEquipmentComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwner())
		OwnerASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwner());
}

void UPCUnitEquipmentComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UPCUnitEquipmentComponent, SlotItemTags);
}

bool UPCUnitEquipmentComponent::TryEquipItem(const FGameplayTag& ItemTag)
{
	return false;
}

void UPCUnitEquipmentComponent::UnionEquipmentComponent(UPCUnitEquipmentComponent* InEquipmentComp)
{
	if (!InEquipmentComp)
		return;
}

void UPCUnitEquipmentComponent::OnRep_SlotItemTags()
{
}

bool UPCUnitEquipmentComponent::CombinationItem(int32 SlotIndex, const FGameplayTag& CombineItemTag)
{
	return false;
}

void UPCUnitEquipmentComponent::ApplyItemEffects(const FGameplayTag& ItemTag, const int32 SlotIndex)
{
	if (!OwnerASC || SlotIndex >= MaxSlotSize)
		return;
	
	UAbilitySystemComponent* ASC = OwnerASC.Get();

	FPCEffectSpecList EffectSpecList;
	ResolveItemEffectSpecList(ItemTag, EffectSpecList);

	if (EffectSpecList.IsEmpty())
		return;

	TArray<FActiveGameplayEffectHandle> ActiveEffectHandles;
	
	for (UPCEffectSpec* EffectSpec : EffectSpecList.EffectSpecs)
	{
		if (!EffectSpec)
			continue;

		FActiveGameplayEffectHandle Handle = EffectSpec->ApplyEffectSelf(ASC);
		if (Handle.IsValid())
			ActiveEffectHandles.Add(Handle);
	}

	if (ActiveEffectHandles.IsEmpty())
	{
		SlotActiveEffects[SlotIndex] = ActiveEffectHandles;
	}
}

void UPCUnitEquipmentComponent::ResolveItemEffectSpecList(const FGameplayTag& ItemTag, FPCEffectSpecList& OutEffectSpecList)
{
}

void UPCUnitEquipmentComponent::RemoveSlotActiveEffect(int32 SlotIndex)
{
}

void UPCUnitEquipmentComponent::ReturnInventoryAllItem()
{
}
