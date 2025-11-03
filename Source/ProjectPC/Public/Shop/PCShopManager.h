// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Shop/PCShopUnitData.h"
#include "Shop/PCShopUnitProbabilityData.h"
#include "Shop/PCShopUnitSellingPriceData.h"
#include "PCShopManager.generated.h"

class APCCombatBoard;
class APCCombatGameState;
class APCHeroUnitCharacter;
class APCPlayerState;

/**
 * 
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTPC_API UPCShopManager : public UActorComponent
{
	GENERATED_BODY()
	
public:
	UPCShopManager();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
#pragma region Shop

private:
	uint8 NumSlots = 5;
	FPCShopUnitData DummyData;

public:
	// GameState Tag가 바뀔 때마다 유닛 레벨업 체크
	void OnGameStateChanged(const FGameplayTag& NewTag);
	
	// 상점 업데이트
	void UpdateShopSlots(APCPlayerState* TargetPlayer);
	
	// 유닛 구매
	void BuyUnit(APCPlayerState* TargetPlayer, int32 SlotIndex, FGameplayTag UnitTag);
	TMap<int32, int32> GetLevelUpUnitMap(const APCPlayerState* TargetPlayer, FGameplayTag UnitTag, int32 ShopAddUnitCount) const;
	int32 GetRequiredCountWithFullBench(const APCPlayerState* TargetPlayer, FGameplayTag UnitTag, int32 ShopAddUnitCount) const;
	void UnitLevelUp(const APCPlayerState* TargetPlayer, FGameplayTag UnitTag, int32 ShopAddUnitCount);

	// 유닛 판매
	void SellUnit(FGameplayTag UnitTag, int32 UnitLevel);

	// 유닛 코스트에 따른 랜덤한 유닛 선택
	FPCShopUnitData& SelectRandomUnitByCost(int32 UnitCost);
	
	// 기물 반환
	void ReturnUnitToShopByTag(FGameplayTag UnitTag);
	void ReturnUnitsToShopByCarousel(const TArray<FGameplayTag>& UnitTags);
	void ReturnUnitsToShopBySlotUpdate(const TArray<FPCShopUnitData>& OldSlots, const TSet<int32>& PurchasedSlots);

	// Carousel에 차출할 유닛 태그 배열 리턴
	TArray<FGameplayTag> GetCarouselUnitTags(int32 Round);
	TArray<FGameplayTag> GetCarouselRandomUnitTagsByCost(int32 UnitCost, int32 CarouselCount);
	
#pragma endregion Shop

#pragma region Data

protected:
	// DataTable
	UPROPERTY(EditAnywhere, Category = "DataTable|Shop")
	TObjectPtr<UDataTable> ShopUnitDataTable;
	UPROPERTY(EditAnywhere, Category = "DataTable|Shop")
	TObjectPtr<UDataTable> ShopUnitProbabilityDataTable;
	UPROPERTY(EditAnywhere, Category = "DataTable|Shop")
	TObjectPtr<UDataTable> ShopUnitSellingPriceDataTable;

	// 실제로 DataTable에서 가져온 정보를 저장할 배열
	TArray<FPCShopUnitData> ShopUnitDataList;
	TArray<FPCShopUnitProbabilityData> ShopUnitProbabilityDataList;
	TMap<TPair<int32, int32>, int32> ShopUnitSellingPriceDataMap;

	// 유닛 코스트별 기물 현황, 실제로 재고 증/차감이 이루어지는 배열
	TArray<FPCShopUnitData> ShopUnitDataList_Cost1;
	TArray<FPCShopUnitData> ShopUnitDataList_Cost2;
	TArray<FPCShopUnitData> ShopUnitDataList_Cost3;
	TArray<FPCShopUnitData> ShopUnitDataList_Cost4;
	TArray<FPCShopUnitData> ShopUnitDataList_Cost5;

public:
	// Getter
	const TArray<FPCShopUnitData>& GetShopUnitDataList();
	const TArray<FPCShopUnitProbabilityData>& GetShopUnitProbabilityDataList();
	const TMap<TPair<int32, int32>, int32>& GetShopUnitSellingPriceDataMap();
	
	int32 GetUnitCostByTag(FGameplayTag UnitTag);
	TArray<FPCShopUnitData>& GetShopUnitDataListByCost(int32 UnitCost);
	const FPCShopUnitData& GetShopUnitDataByTag(FGameplayTag UnitTag);

	TArray<float> GetCostProbabilities(int32 PlayerLevel);

	int32 GetSellingPrice(int32 UnitCost, int32 UnitLevel);
	
#pragma endregion Data
	
#pragma region TemplateFunc
	
private:
	// DataTable을 읽어 아웃파라미터로 TArray에 값을 넘기는 템플릿 함수
	template<typename T>
	void LoadDataTable(UDataTable* DataTable, TArray<T>& OutDataList, const FString& Context)
	{
		if (DataTable == nullptr) return;
		OutDataList.Reset();

		TArray<T*> RowPtrs;
		DataTable->GetAllRows(Context, RowPtrs);

		// DataTable의 Row수만큼 메모리 미리 확보
		OutDataList.Reserve(RowPtrs.Num());
		for (const auto Row : RowPtrs)
		{
			if (Row)
			{
				OutDataList.Add(*Row);
			}
		}
	}

	// DataTable을 읽어 아웃파라미터로 TMap에 값을 넘기는 템플릿 함수
	template<typename T>
	void LoadDataTableToMap(UDataTable* DataTable, TMap<TPair<int32, int32>, int32>& OutMap, const FString& Context)
	{
		if (DataTable == nullptr) return;
		OutMap.Reset();

		TArray<T*> RowPtrs;
		DataTable->GetAllRows(Context, RowPtrs);

		for (const auto Row : RowPtrs)
		{
			if (Row)
			{
				// Key는 (UnitCost, UnitLevel), 값은 UnitSellingPrice
				TPair<int32, int32> Key(Row->UnitCost, Row->UnitLevel);
				OutMap.Add(Key, Row->UnitSellingPrice);
			}
		}
	}
	
	// 누적합을 통한 확률 구현
	template<typename T>
	const T& WeightedRandomSelect(const TArray<T>& Items, T MinValue, T MaxValue, int32& Index)
	{
		T RandomValue = FMath::RandRange(MinValue, MaxValue);
		T PrefixSum = MinValue;

		for (const auto& Item : Items)
		{
			PrefixSum += Item;
			if (RandomValue < PrefixSum)
			{
				return Item;
			}

			++Index;
		}

		return Items.Last();
	}

#pragma endregion TemplateFunc
};

