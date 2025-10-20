// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFramework/WorldSubsystem/PCItemManagerSubsystem.h"

#include "BaseGameplayTags.h"


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

FGameplayTag UPCItemManagerSubsystem::CombineItem(FGameplayTag ItemTag1, FGameplayTag ItemTag2) const
{
	const FGameplayTag ItemTypeTag = FGameplayTag::RequestGameplayTag(FName("Item.Type.Base"));
	
	if (ItemTag1.MatchesTag(ItemTypeTag) && ItemTag2.MatchesTag(ItemTypeTag))
	{
		if (const auto AdvancedItemTag = ItemCombineDataMap.Find(FBaseItemPair(ItemTag1, ItemTag2)))
		{
			return *AdvancedItemTag;
		}
	}
	
	return FGameplayTag();
}

TMap<FGameplayTag, FGameplayTag> UPCItemManagerSubsystem::GetItemRecipe(FGameplayTag BaseItemTag) const
{
	TMap<FGameplayTag, FGameplayTag> ItemRecipes;
	
	FGameplayTag ParentItemTag = FGameplayTag::RequestGameplayTag(FName("Item.Type.Base"));
	if (!BaseItemTag.MatchesTag(ParentItemTag))
		return ItemRecipes;
	
	UGameplayTagsManager& TagManager = UGameplayTagsManager::Get();
	FGameplayTagContainer AllBaseItemTags = TagManager.RequestGameplayTagChildren(ParentItemTag);

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

	if (AllBaseItemTags.Num() > 0)
	{
		int32 RandomIndex = FMath::RandRange(0, AllBaseItemTags.Num() - 1);

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

	if (AllAdvancedItemTags.Num() > 0)
	{
		int32 RandomIndex = FMath::RandRange(0, AllAdvancedItemTags.Num() - 1);

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
