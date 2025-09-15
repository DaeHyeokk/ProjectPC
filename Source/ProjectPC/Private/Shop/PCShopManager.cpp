// Fill out your copyright notice in the Description page of Project Settings.


#include "Shop/PCShopManager.h"

#include "Engine/DataTable.h"

#include "GameFramework/GameState/PCCombatGameState.h"
#include "GameFramework/PlayerState/PCPlayerState.h"
#include "GameFramework/WorldSubsystem/PCUnitSpawnSubsystem.h"
#include "AbilitySystem/Player/AttributeSet/PCPlayerAttributeSet.h"
#include "GameFramework/HelpActor/PCCombatBoard.h"
#include "GameFramework/HelpActor/Component/PCTileManager.h"


class UPCUnitSpawnSubsystem;

void UPCShopManager::UpdateShopSlots(APCPlayerState* TargetPlayer)
{
	if (!TargetPlayer) return;
	
	auto GS = Cast<APCCombatGameState>(GetOwner());
	if (!GS) return;
	
	const auto& ShopSlots = TargetPlayer->GetShopSlots();
	GS->ReturnUnitsToShopBySlotUpdate(ShopSlots, TargetPlayer->PurchasedSlots);
	TargetPlayer->PurchasedSlots.Empty();

	TArray<FPCShopUnitData> NewShopSlots;
	const auto PlayerLevel = static_cast<int32>(TargetPlayer->GetAttributeSet()->GetPlayerLevel());
	const auto& CostProbabilities = GS->GetCostProbabilities(PlayerLevel);
	
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

void UPCShopManager::BuyUnit(APCPlayerState* TargetPlayer, int32 SlotIndex, FGameplayTag UnitTag, int32 BenchIndex)
{
	if (!TargetPlayer) return;

	auto GS = Cast<APCCombatGameState>(GetOwner());
	if (!GS) return;

	auto Board = GS->GetBoardBySeat(TargetPlayer->SeatIndex);
	if (!Board) return;
	
	auto Unit = GetWorld()->GetSubsystem<UPCUnitSpawnSubsystem>()->SpawnUnitByTag(UnitTag, TargetPlayer->SeatIndex);
	Board->TileManager->PlaceUnitOnBench(BenchIndex, Unit);

	TargetPlayer->PurchasedSlots.Add(SlotIndex);
}

void UPCShopManager::SellUnit(FGameplayTag UnitTag, int32 UnitLevel)
{
	auto GS = Cast<APCCombatGameState>(GetOwner());
	if (!GS) return;

	auto UnitCost = GS->GetUnitCostByTag(UnitTag);

	for (auto& Unit : GS->GetShopUnitDataListByCost(UnitCost))
	{
		if (UnitTag == Unit.Tag)
		{
			// 1성은 1개, 2성은 3개, 3성은 9개 기물 반환
				// 4성이 추가되도 그대로 사용 가능
			Unit.UnitCount += FMath::Pow(3.f, static_cast<float>(UnitLevel) - 1.f);
		}
	}
}
