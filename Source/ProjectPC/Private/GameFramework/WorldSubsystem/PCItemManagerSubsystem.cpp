// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFramework/WorldSubsystem/PCItemManagerSubsystem.h"


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

const FPCItemData* UPCItemManagerSubsystem::CombineItem(FGameplayTag ItemTag1, FGameplayTag ItemTag2) const
{
	if (const auto AdvancedItemTag = ItemCombineDataMap.Find(FBaseItemPair(ItemTag1, ItemTag2)))
	{
		return GetItemData(*AdvancedItemTag);
	}

	return nullptr;
}
