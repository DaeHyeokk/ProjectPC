// Fill out your copyright notice in the Description page of Project Settings.


#include "Shop/PCShopManager.h"

#include "Engine/DataTable.h"

#include "GameFramework/GameState/PCCombatGameState.h"
#include "GameFramework/PlayerState/PCPlayerState.h"


void UPCShopManager::UpdateShopSlots(APCPlayerState* TargetPlayer)
{
	auto GS = GetWorld()->GetGameState<APCCombatGameState>();
	if (!GS) return;
	
	const auto& ShopSlots = TargetPlayer->GetShopSlots();
	ReturnUnitsToShop(GS, ShopSlots);

	TArray<FPCShopUnitData> NewShopSlots;
	const auto& CostProbabilities = GS->GetCostProbabilities();
	
	for (uint8 i = 0; i < NumSlots; ++i)
	{
		// 누적합 범위에 따라 코스트 선택
		int32 SelectedCost = 1;
		WeightedRandomSelect<float>(CostProbabilities, 0.f, 1.f, SelectedCost);

		// 상점에 남은 해당 코스트 모든 유닛 후보로 추가
		auto& Candidates = GS->GetShopUnitDataListByCost(SelectedCost);
		TArray<int32> UnitCounts;
		int32 TotalUnitCount = 0;
		for (auto& Unit : Candidates)
		{
			UnitCounts.Add(Unit.UnitCount);
			TotalUnitCount += Unit.UnitCount;
		}

		// 해당 코스트에 아무 기물도 존재하지 않을 때
		if (TotalUnitCount == 0)
		{
			--i;
			UE_LOG(LogTemp, Warning, TEXT("SelectedCost : %d Sold Out"), SelectedCost);
			continue;
		}
		
		// 누적합 범위에 따라 기물 선택
		int32 SelectedUnit = 0;
		WeightedRandomSelect<int32>(UnitCounts, 0, TotalUnitCount - 1, SelectedUnit);
		Candidates[SelectedUnit].UnitCount -= 1;
		NewShopSlots.Add(Candidates[SelectedUnit]);
	}

	TargetPlayer->SetShopSlots(NewShopSlots);
}

void UPCShopManager::ReturnUnitsToShop(APCCombatGameState* GS, const TArray<FPCShopUnitData>& OldSlots)
{
	// 구매하지 않은 유닛 상점에 기물 반환
	for (const auto& OldSlot : OldSlots)
	{
		for (auto& Unit : GS->GetShopUnitDataListByCost(OldSlot.UnitCost))
		{
			if (Unit.UnitName == OldSlot.UnitName)
			{
				Unit.UnitCount += 1;
				break;
			}
		}
	}
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
