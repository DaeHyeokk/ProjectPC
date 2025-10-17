// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Item/PCItemData.h"
#include "Item/PCItemCombineData.h"
#include "PCItemManagerSubsystem.generated.h"

USTRUCT(BlueprintType)
struct FBaseItemPair
{
	GENERATED_BODY();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag ItemTag1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag ItemTag2;

	bool operator==(const FBaseItemPair& Other) const
	{
		return ItemTag1 == Other.ItemTag1 && ItemTag2 == Other.ItemTag2;
	}
};

FORCEINLINE uint32 GetTypeHash(const FBaseItemPair& Pair)
{
	return HashCombine(GetTypeHash(Pair.ItemTag1), GetTypeHash(Pair.ItemTag2));
}

/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCItemManagerSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
protected:
	TMap<FGameplayTag, FPCItemData> ItemDataMap;
	TMap<FBaseItemPair, FGameplayTag> ItemCombineDataMap;
	
public:
	void InitializeItemManager(UDataTable* ItemDataTable, UDataTable* ItemCombineDataTable);

	const FPCItemData* GetItemData(FGameplayTag ItemTag) const;
	FGameplayTag CombineItem(FGameplayTag ItemTag1, FGameplayTag ItemTag2) const;

#pragma region TemplateFunc

private:
	// DataTable을 읽어 아웃파라미터로 TMap에 값을 넘기는 템플릿 함수
	template<typename KeyType, typename ValueType, typename RowType, typename KeyExtractorType, typename ValueExtractorType>
	void LoadDataTableToMap(
		UDataTable* DataTable,
		TMap<KeyType, ValueType>& OutMap,
		KeyExtractorType KeyExtractor,
		ValueExtractorType ValueExtractor,
		const FString& Context)
	{
		if (DataTable == nullptr) return;
		OutMap.Reset();

		TArray<RowType*> RowPtrs;
		DataTable->GetAllRows(Context, RowPtrs);

		for (const auto Row : RowPtrs)
		{
			if (Row)
			{
				KeyType Key = KeyExtractor(Row);
				ValueType Value = ValueExtractor(Row);
				OutMap.Add(Key, Value);
			}
		}
	}
	
#pragma endregion TemplateFunc
};
