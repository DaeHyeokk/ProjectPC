// Fill out your copyright notice in the Description page of Project Settings.


#include "Shop/PCShopManager.h"

#include "Engine/DataTable.h"

#include "GameFramework/GameState/PCCombatGameState.h"


void UPCShopManager::UpdateShopSlots()
{
	auto GS = GetWorld()->GetGameState<APCCombatGameState>();
	if (!GS) return;
	
	for (const FPCShopUnitData& OldSlot : ShopSlots)
	{
		for (FPCShopUnitData& Unit : GS->GetShopUnitDataList())
		{
			if (Unit.UnitName == OldSlot.UnitName)
			{
				Unit.UnitCount += 1;
				break;
			}
		}
	}
	
	ShopSlots.Empty();
	
	uint8 NumSlots = 5;
	TArray<float> CostProbabilities = GS->GetCostProbabilities();
	
	for (uint8 i = 0; i < NumSlots; ++i)
	{
		// 누적합 범위에 따라 코스트 선택
		int32 SelectedCost = 1;
		WeightedRandomSelect<float>(CostProbabilities, 0.f, 1.f, SelectedCost);

		// 상점에 남은 해당 코스트 모든 유닛 후보로 추가
		TArray<FPCShopUnitData*> Candidates;
		TArray<int32> UnitCounts;
		int32 TotalUnitCount = 0;
		for (auto& Unit : GS->GetShopUnitDataList())
		{
			if (Unit.UnitCost == SelectedCost && Unit.UnitCount > 0)
			{
				Candidates.Add(&Unit);
				UnitCounts.Add(Unit.UnitCount);
				TotalUnitCount += Unit.UnitCount;
			}
		}

		// 해당 코스트에 아무 기물도 존재하지 않을 때
		if (Candidates.Num() == 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("SelectedCost : %d Sold Out"), SelectedCost);
			--i;
			continue;
		}
		
		// 누적합 범위에 따라 기물 선택
		int32 SelectedUnit = 0;
		WeightedRandomSelect<int32>(UnitCounts, 0, TotalUnitCount - 1, SelectedUnit);
		ShopSlots.Add(*Candidates[SelectedUnit]);
		Candidates[SelectedUnit]->UnitCount -= 1;
	}
}

const TArray<FPCShopUnitData>& UPCShopManager::GetShopSlots()
{
	return ShopSlots;
}

void UPCShopManager::BuyXP()
{
}

void UPCShopManager::BuyUnit()
{
}

void UPCShopManager::SellUnit()
{
}

void UPCShopManager::ShopLock()
{
}
