// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/PCUnitEquipmentComponent.h"

#include "AbilitySystemComponent.h"
#include "BaseGameplayTags.h"
#include "AbilitySystem/Unit/EffectSpec/PCEffectSpec.h"
#include "Character/Unit/PCBaseUnitCharacter.h"
#include "GameFramework/PlayerState/PCPlayerState.h"
#include "GameFramework/WorldSubsystem/PCItemManagerSubsystem.h"
#include "Item/PCPlayerInventory.h"
#include "Net/UnrealNetwork.h"

UPCUnitEquipmentComponent::UPCUnitEquipmentComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);

	MaxSlotSize = 3;
	SlotItemTags.SetNum(MaxSlotSize);
	SlotActiveEffects.SetNum(MaxSlotSize);
}

void UPCUnitEquipmentComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwner())
	{
		Owner = Cast<APCBaseUnitCharacter>(GetOwner());
		
		if (APCBaseUnitCharacter* Unit = Cast<APCBaseUnitCharacter>(GetOwner()))
		{
			if (APCPlayerState* PS = Unit->GetOwnerPlayerState())
			{
				OwnerPlayerInventory = PS->GetPlayerInventory();
			}
		}
	}
	
	if (GetWorld())
	{
		ItemManagerSubsystem = GetWorld()->GetSubsystem<UPCItemManagerSubsystem>();
	}
}

void UPCUnitEquipmentComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UPCUnitEquipmentComponent, SlotItemTags);
}

bool UPCUnitEquipmentComponent::TryEquipItem(const FGameplayTag& ItemTag, bool bIsUnion)
{
	if (!HasAuthority() || !ItemManagerSubsystem.IsValid() || !ItemTag.IsValid())
		return false;

	// 베이스 아이템일 경우 조합 가능 여부 확인
	if (ItemTag.MatchesTag(ItemTags::Item_Type_Base))
	{
		for (int32 i = 0; i<MaxSlotSize; ++i)
		{
			FGameplayTag& SlotItemTag = SlotItemTags[i];
			if (!SlotItemTag.IsValid())
				break;
			
			if (SlotItemTag.MatchesTag(ItemTags::Item_Type_Base))
			{
				const FGameplayTag CombineResultItemTag = ItemManagerSubsystem->CombineItem(ItemTag, SlotItemTag);
				// 조합 성공 시
				if (CombineResultItemTag.IsValid())
				{
					SetItemToSlot(CombineResultItemTag, i, !bIsUnion);
					return true;
				}
			}
		}
	}
	
	int32 EmptySlotIndex = INDEX_NONE;
	for (int32 i = 0; i < MaxSlotSize; ++i)
	{
		if (!SlotItemTags[i].IsValid())
		{
			EmptySlotIndex = i;
			break;
		}
	}

	if (EmptySlotIndex != INDEX_NONE)
	{
		SetItemToSlot(ItemTag, EmptySlotIndex, !bIsUnion);
		return true;
	}

	return false;
}

void UPCUnitEquipmentComponent::UnionEquipmentComponent(UPCUnitEquipmentComponent* InEquipmentComp)
{
	if (!HasAuthority() || !InEquipmentComp)
		return;

	for (int32 i=0; i<MaxSlotSize; ++i)
	{
		FGameplayTag& SlotItemTag = InEquipmentComp->SlotItemTags[i];
		
		if (!SlotItemTag.IsValid())
			break;

		// 아이템 장착에 실패할 경우 플레이어 인벤토리로 넘어감
		if (!TryEquipItem(SlotItemTag, true))
		{
			ReturnItemToPlayerInventory(SlotItemTag);
		}

		InEquipmentComp->RemoveItemSlot(i);
	}
}

void UPCUnitEquipmentComponent::OnRep_SlotItemTags() const
{
	OnEquipItemChanged.Broadcast();
}

void UPCUnitEquipmentComponent::SetItemToSlot(const FGameplayTag& ItemTag, const int32 SlotIndex, const bool bPlayParticle)
{
	if (!HasAuthority())
		return;
	
	if (SlotItemTags.IsValidIndex(SlotIndex))
	{
		if (SlotItemTags[SlotIndex].IsValid())
		{
			RemoveSlotActiveEffects(SlotIndex);
		}
		
		SlotItemTags[SlotIndex] = ItemTag;
		ApplyItemEffects(ItemTag, SlotIndex);

		if (bPlayParticle)
		{
			if (UAbilitySystemComponent* OwnerASC = Owner->GetAbilitySystemComponent())
			{
				FGameplayCueParameters Params;
				Params.TargetAttachComponent = Owner->GetMesh();
				OwnerASC->ExecuteGameplayCue(GameplayCueTags::GameplayCue_VFX_Unit_EquipItem, Params);
			}
		}
	}
}

void UPCUnitEquipmentComponent::RemoveItemSlot(const int32 SlotIndex)
{
	if (!HasAuthority() || !SlotItemTags.IsValidIndex(SlotIndex))
		return;

	if (Owner.IsValid() && !SlotActiveEffects[SlotIndex].IsEmpty())
	{
		for (const FActiveGameplayEffectHandle& EffectHandle : SlotActiveEffects[SlotIndex])
		{
			if (!EffectHandle.IsValid())
				continue;

			if (UAbilitySystemComponent* OwnerASC = Owner->GetAbilitySystemComponent())
				OwnerASC->RemoveActiveGameplayEffect(EffectHandle);
		}
	}

	SlotActiveEffects[SlotIndex].Reset();
	SlotItemTags[SlotIndex] = FGameplayTag::EmptyTag;
}

void UPCUnitEquipmentComponent::ReturnAllItemToPlayerInventory(const bool bIsDestroyedHero)
{
	if (!HasAuthority() || !OwnerPlayerInventory.IsValid())
		return;

	for (int32 i=0; i<MaxSlotSize; ++i)
	{
		FGameplayTag SlotItemTag = SlotItemTags[i];
		if (!SlotItemTag.IsValid())
			return;

		if (!bIsDestroyedHero)
			RemoveItemSlot(i);
		
		OwnerPlayerInventory->AddItemToInventory(SlotItemTag);
	}
}

void UPCUnitEquipmentComponent::ReturnItemToPlayerInventory(const FGameplayTag& ItemTag) const
{
	if (!HasAuthority() || !OwnerPlayerInventory.IsValid() || !ItemTag.IsValid())
		return;

	OwnerPlayerInventory->AddItemToInventory(ItemTag);
}

void UPCUnitEquipmentComponent::ApplyItemEffects(const FGameplayTag& ItemTag, const int32 SlotIndex)
{
	if (!HasAuthority() || !ItemTag.IsValid() || !SlotItemTags.IsValidIndex(SlotIndex))
		return;

	const FPCEffectSpecList* EffectSpecList = ResolveItemEffectSpecList(ItemTag);

	if (!EffectSpecList || EffectSpecList->IsEmpty())
		return;

	TArray<FActiveGameplayEffectHandle> ActiveEffectHandles;
	
	for (UPCEffectSpec* EffectSpec : EffectSpecList->EffectSpecs)
	{
		if (!EffectSpec)
			continue;

		if (UAbilitySystemComponent* OwnerASC = Owner.IsValid() ? Owner->GetAbilitySystemComponent() : nullptr)
		{
			FActiveGameplayEffectHandle Handle = EffectSpec->ApplyEffectSelf(OwnerASC);
			if (Handle.IsValid())
				ActiveEffectHandles.Add(Handle);
		}
	}

	if (!ActiveEffectHandles.IsEmpty())
	{
		SlotActiveEffects[SlotIndex] = ActiveEffectHandles;
	}
}

void UPCUnitEquipmentComponent::RemoveSlotActiveEffects(const int32 SlotIndex)
{
	if (!HasAuthority() || !SlotActiveEffects.IsValidIndex(SlotIndex))
		return;
	
	if (UAbilitySystemComponent* OwnerASC = Owner->GetAbilitySystemComponent())
	{
		for (FActiveGameplayEffectHandle& EffectHandle : SlotActiveEffects[SlotIndex])
		{
			if (!EffectHandle.IsValid())
				continue;
		
			OwnerASC->RemoveActiveGameplayEffect(EffectHandle);
		}
	}
	
	SlotActiveEffects[SlotIndex].Reset();
}

const FPCEffectSpecList* UPCUnitEquipmentComponent::ResolveItemEffectSpecList(const FGameplayTag& ItemTag) const
{
	if (!HasAuthority() || !ItemTag.IsValid() || !ItemManagerSubsystem.IsValid())
		return nullptr;

	return ItemManagerSubsystem->GetItemEffectSpecList(ItemTag);
}

bool UPCUnitEquipmentComponent::HasAuthority() const
{
	return Owner.IsValid() && Owner->HasAuthority();
}
