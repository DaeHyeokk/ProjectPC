// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFramework/WorldSubsystem/PCItemManagerSubsystem.h"

#include "BaseGameplayTags.h"
#include "DataAsset/Item/PCDataAsset_ItemEffect.h"


void UPCItemManagerSubsystem::InitializeItemManager(UDataTable* ItemDataTable, UDataTable* ItemCombineDataTable)
{
	if (ItemDataTable && ItemCombineDataTable)
	{
		LoadDataTableToMap<FGameplayTag, FPCItemData, FPCItemData>(
			ItemDataTable, ItemDataMap,
			[](const FPCItemData* Row) { return Row->ItemTag; },
			[](const FPCItemData* Row) { return *Row; },
			TEXT("Loading Item Data"));

		LoadDataTableToMap<FBaseItemPair, FGameplayTag, FPCItemCombineData>(
			ItemCombineDataTable, ItemCombineDataMap,
			[](const FPCItemCombineData* Row) { return FBaseItemPair(Row->ItemTag1, Row->ItemTag2); },
			[](const FPCItemCombineData* Row) { return Row->ResultItemTag; },
			TEXT("Loading Item Combine Data"));
	}
}

const FPCItemData* UPCItemManagerSubsystem::GetItemData(FGameplayTag ItemTag) const
{
	return ItemDataMap.Find(ItemTag);
}

const FPCEffectSpecList* UPCItemManagerSubsystem::GetItemEffectSpecList(FGameplayTag ItemTag) const
{
	if (const FPCItemData* ItemData = GetItemData(ItemTag))
	{
		if (const UPCDataAsset_ItemEffect* ItemEffectData = ItemData->ItemEffectSpec)
		{
			return &ItemEffectData->EffectSpecList;
		}
	}

	return nullptr;
}

TMap<FGameplayTag, FGameplayTag> UPCItemManagerSubsystem::GetItemRecipe(FGameplayTag BaseItemTag) const
{
	TMap<FGameplayTag, FGameplayTag> ItemRecipes;
	
	FGameplayTag ParentItemTag = FGameplayTag::RequestGameplayTag(FName("Item.Type.Base"));
	if (!BaseItemTag.MatchesTag(ParentItemTag))
		return ItemRecipes;
	
	UGameplayTagsManager& TagManager = UGameplayTagsManager::Get();
	FGameplayTagContainer AllBaseItemTags = TagManager.RequestGameplayTagChildren(ParentItemTag);

	// 재료 아이템 타입 모두와 조합한 결과를 TMap에 저장하여 리턴
	for (const FGameplayTag& ItemTag : AllBaseItemTags)
	{
		FGameplayTag AdvancedItemTag = CombineItem(BaseItemTag, ItemTag);

		if (const auto NewItem = GetItemData(AdvancedItemTag))
		{
			if (NewItem->IsValid())
			{
				ItemRecipes.Add(ItemTag, AdvancedItemTag);
			}
		}
	}

	return ItemRecipes;
}

FGameplayTag UPCItemManagerSubsystem::GetRandomBaseItem() const
{
	FGameplayTag ParentItemTag = FGameplayTag::RequestGameplayTag(FName("Item.Type.Base"));
	
	UGameplayTagsManager& TagManager = UGameplayTagsManager::Get();
	FGameplayTagContainer AllBaseItemTags = TagManager.RequestGameplayTagChildren(ParentItemTag);
	AllBaseItemTags.RemoveTag(ParentItemTag);

	// 랜덤 인덱스를 뽑아서 해당 인덱스의 제료 아이템이 유효하면 ItemTag return
	if (AllBaseItemTags.Num() > 0)
	{
		int32 RandomIndex = FMath::RandHelper(AllBaseItemTags.Num());

		if (const auto NewItem = GetItemData(AllBaseItemTags.GetByIndex(RandomIndex)))
		{
			if (NewItem->IsValid())
			{
				return NewItem->ItemTag;
			}
		}
	}
	
	return FGameplayTag();
}

FGameplayTag UPCItemManagerSubsystem::GetRandomAdvancedItem() const
{
	FGameplayTag ParentItemTag = FGameplayTag::RequestGameplayTag(FName("Item.Type.Advanced"));
	
	UGameplayTagsManager& TagManager = UGameplayTagsManager::Get();
	FGameplayTagContainer AllAdvancedItemTags = TagManager.RequestGameplayTagChildren(ParentItemTag);
	AllAdvancedItemTags.RemoveTag(ParentItemTag);

	// 랜덤 인덱스를 뽑아서 해당 인덱스의 완성 아이템이 유효하면 ItemTag 리턴
	if (AllAdvancedItemTags.Num() > 0)
	{
		int32 RandomIndex = FMath::RandHelper(AllAdvancedItemTags.Num());

		if (const auto NewItem = GetItemData(AllAdvancedItemTags.GetByIndex(RandomIndex)))
		{
			if (NewItem->IsValid())
			{
				return NewItem->ItemTag;
			}
		}
	}
	
	return FGameplayTag();
}

FGameplayTag UPCItemManagerSubsystem::CombineItem(FGameplayTag ItemTag1, FGameplayTag ItemTag2) const
{
	const FGameplayTag ItemTypeTag = FGameplayTag::RequestGameplayTag(FName("Item.Type.Base"));
	
	if (ItemTag1.MatchesTag(ItemTypeTag) && ItemTag2.MatchesTag(ItemTypeTag))
	{
		// ItemCombineDataMap에 <ItemTag1, ItemTag2> 쌍의 Key값이 존재하면 Value 리턴
		if (const auto AdvancedItemTag = ItemCombineDataMap.Find(FBaseItemPair(ItemTag1, ItemTag2)))
		{
			return *AdvancedItemTag;
		}
	}
	
	return FGameplayTag();
}

UTexture2D* UPCItemManagerSubsystem::GetItemTexture(const FGameplayTag& ItemTag)
{
	if (!ItemTag.IsValid() || !ItemTag.MatchesTag(ItemTags::Item))
		return nullptr;
	
	UTexture2D* ItemTexture = ItemTextureMap.FindRef(ItemTag);
	if (!ItemTexture)
	{
		if (const FPCItemData* ItemData = ItemDataMap.Find(ItemTag))
		{
			ItemTexture = ItemData->ItemTexture.LoadSynchronous();
			if (ItemTexture)
			{
				ItemTextureMap.Add(ItemTag, ItemTexture);
			}
		}
	}

	return ItemTexture;
}
