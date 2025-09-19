// Fill out your copyright notice in the Description page of Project Settings.


#include "Shop/PCShopManager.h"

#include "Engine/DataTable.h"

#include "BaseGameplayTags.h"
#include "GameFramework/GameState/PCCombatGameState.h"
#include "GameFramework/PlayerState/PCPlayerState.h"
#include "GameFramework/WorldSubsystem/PCUnitSpawnSubsystem.h"
#include "AbilitySystem/Player/AttributeSet/PCPlayerAttributeSet.h"
#include "Character/Unit/PCBaseUnitCharacter.h"
#include "Character/Unit/PCHeroUnitCharacter.h"
#include "GameFramework/HelpActor/PCCombatBoard.h"
#include "GameFramework/HelpActor/Component/PCTileManager.h"

class UPCUnitSpawnSubsystem;

UPCShopManager::UPCShopManager()
{
	DummyData.UnitName = "Dummy";
}

void UPCShopManager::BeginPlay()
{
	Super::BeginPlay();
	
	if (ShopUnitDataTable && ShopUnitProbabilityDataTable && ShopUnitSellingPriceDataTable)
	{
		LoadDataTable<FPCShopUnitData>(ShopUnitDataTable, ShopUnitDataList, TEXT("Loading Shop Unit Data"));
		LoadDataTable<FPCShopUnitProbabilityData>(ShopUnitProbabilityDataTable, ShopUnitProbabilityDataList, TEXT("Loading Shop Unit Probability Data"));
		LoadDataTableToMap<FPCShopUnitSellingPriceData>(ShopUnitSellingPriceDataTable, ShopUnitSellingPriceDataMap, TEXT("Loading Shop Unit Selling Price Data"));
	}

	for (auto Unit : ShopUnitDataList)
	{
		switch (Unit.UnitCost)
		{
		case 1:
			ShopUnitDataList_Cost1.Add(Unit);
			break;
		case 2:
			ShopUnitDataList_Cost2.Add(Unit);
			break;
		case 3:
			ShopUnitDataList_Cost3.Add(Unit);
			break;
		case 4:
			ShopUnitDataList_Cost4.Add(Unit);
			break;
		case 5:
			ShopUnitDataList_Cost5.Add(Unit);
			break;
		default:
			break;
		}
	}
}

void UPCShopManager::UpdateShopSlots(APCPlayerState* TargetPlayer)
{
	if (!TargetPlayer) return;
	
	const auto& ShopSlots = TargetPlayer->GetShopSlots();
	ReturnUnitsToShopBySlotUpdate(ShopSlots, TargetPlayer->PurchasedSlots);
	TargetPlayer->PurchasedSlots.Empty();

	TArray<FPCShopUnitData> NewShopSlots;
	const auto PlayerLevel = static_cast<int32>(TargetPlayer->GetAttributeSet()->GetPlayerLevel());
	const auto& CostProbabilities = GetCostProbabilities(PlayerLevel);
	
	for (uint8 i = 0; i < NumSlots; ++i)
	{
		// 누적합 범위에 따라 코스트 선택
		int32 SelectedCost = 1;
		WeightedRandomSelect<float>(CostProbabilities, 0.f, 1.f, SelectedCost);

		auto& Candidate =  SelectRandomUnitByCost(SelectedCost);

		// 해당 코스트에 아무 기물도 존재하지 않을 때
		if (Candidate.UnitName == "Dummy")
		{
			--i;
			UE_LOG(LogTemp, Warning, TEXT("SelectedCost : %d Sold Out"), SelectedCost);
			continue;
		}
		
		NewShopSlots.Add(Candidate);
	}
	
	TargetPlayer->SetShopSlots(NewShopSlots);
}

void UPCShopManager::BuyUnit(APCPlayerState* TargetPlayer, int32 SlotIndex, FGameplayTag UnitTag, const int32 BenchIndex)
{
	if (!TargetPlayer) return;

	auto GS = Cast<APCCombatGameState>(GetOwner());
	if (!GS) return;

	auto Board = GS->GetBoardBySeat(TargetPlayer->SeatIndex);
	if (!Board) return;
	
	auto Unit = GetWorld()->GetSubsystem<UPCUnitSpawnSubsystem>()->SpawnUnitByTag(UnitTag, TargetPlayer->SeatIndex);
	Board->TileManager->PlaceUnitOnBench(BenchIndex, Unit);
	UnitLevelUp(TargetPlayer, UnitTag);
	
	TargetPlayer->PurchasedSlots.Add(SlotIndex);
}

TMap<int32, int32> UPCShopManager::GetLevelUpUnitMap(const APCPlayerState* TargetPlayer, FGameplayTag UnitTag, int32 ShopAddUnitCount) const
{
	TMap<int32, int32> UnitCountByLevelMap;
	
	if (!TargetPlayer) return UnitCountByLevelMap;

	auto GS = Cast<APCCombatGameState>(GetOwner());
	if (!GS) return UnitCountByLevelMap;

	auto Board = GS->GetBoardBySeat(TargetPlayer->SeatIndex);
	if (!Board) return UnitCountByLevelMap;
	auto TileManager = Board->TileManager;

	TArray<APCBaseUnitCharacter*> UnitList;
	auto CurrentGameStateTag = GS->GetGameStateTag();

	// 실제 게임 적용할 때는 주석 해제
	// if (CurrentGameStateTag == GameStateTags::Game_State_NonCombat)
	// {
	// 	UnitList = TileManager->GetAllUnitByTag(UnitTag);
	// }
	// else
	// {
	// 	UnitList = TileManager->GetBenchUnitByTag(UnitTag);
	// }

	// 테스트용, 실제 게임 적용할 때는 삭제
	UnitList = TileManager->GetAllUnitByTag(UnitTag);
	
	for (auto Unit : UnitList)
	{
		if (auto HeroUnit = Cast<APCHeroUnitCharacter>(Unit))
		{
			UnitCountByLevelMap.FindOrAdd(HeroUnit->GetUnitLevel())++;
		}
	}

	if (ShopAddUnitCount >= 1 && ShopAddUnitCount <= 2)
	{
		UnitCountByLevelMap.FindOrAdd(1) += ShopAddUnitCount;
	}
	if (ShopAddUnitCount == 3)
	{
		UnitCountByLevelMap.FindOrAdd(2) += 1;
	}
	
	return UnitCountByLevelMap;
}

int32 UPCShopManager::GetRequiredCountWithFullBench(const APCPlayerState* TargetPlayer, FGameplayTag UnitTag, int32 ShopAddUnitCount) const
{
	if (!TargetPlayer) return false;

	auto UnitMap = GetLevelUpUnitMap(TargetPlayer, UnitTag, 0);

	int32 Level1Count = UnitMap.FindRef(1);
	int32 Level2Count = UnitMap.FindRef(2);

	if (Level1Count + ShopAddUnitCount >= 3)
	{
		if (Level1Count == 1)
		{
			return 2;
		}

		if (Level1Count == 2)
		{
			return 1;
		}
	}

	if (Level2Count == 2 && ShopAddUnitCount >= 3)
	{
		return 3;
	}

	return 0; 
}

void UPCShopManager::UnitLevelUp(const APCPlayerState* TargetPlayer, FGameplayTag UnitTag)
{
	if (!TargetPlayer) return;
	
	auto GS = Cast<APCCombatGameState>(GetOwner());
	if (!GS) return;

	auto Board = GS->GetBoardBySeat(TargetPlayer->SeatIndex);
	if (!Board) return;
	auto TileManager = Board->TileManager;
	
	bool bIsOngoing = true;
	while (bIsOngoing)
	{
		bIsOngoing = false;
		
		auto UnitCountByLevelMap = GetLevelUpUnitMap(TargetPlayer, UnitTag, 0);
		
		for (auto& Pair : UnitCountByLevelMap)
		{
			auto UnitLevel = Pair.Key;
			auto UnitCount = Pair.Value;

			if (UnitCount >= 3)
			{
				TArray<APCHeroUnitCharacter*> HeroUnitList;
				for (auto Unit : TileManager->GetAllUnitByTag(UnitTag))
				{
					auto HeroUnit = Cast<APCHeroUnitCharacter>(Unit);
					if (!IsValid(HeroUnit) || HeroUnit->IsActorBeingDestroyed())
					{
						continue;
					}
					
					if (HeroUnit->GetUnitLevel() == UnitLevel)
					{
						HeroUnitList.Add(HeroUnit);
						if (HeroUnitList.Num() == 3)
						{
							break;
						}
					}
				}

				if (HeroUnitList.Num() == 3)
				{
					auto LevelUpUnit = HeroUnitList[0];
					LevelUpUnit->SetUnitLevel(UnitLevel + 1);

					for (int i = 1; i < 3; i++)
					{
						TileManager->RemoveFromBoard(HeroUnitList[i]);
						HeroUnitList[i]->Destroy();
					}
				}

				bIsOngoing = true;
				break;
			}
		}
	}
}

void UPCShopManager::SellUnit(FGameplayTag UnitTag, int32 UnitLevel)
{
	auto GS = Cast<APCCombatGameState>(GetOwner());
	if (!GS) return;

	auto UnitCost = GetUnitCostByTag(UnitTag);

	for (auto& Unit : GetShopUnitDataListByCost(UnitCost))
	{
		if (UnitTag == Unit.Tag)
		{
			// 1성은 1개, 2성은 3개, 3성은 9개 기물 반환
				// 4성이 추가되도 그대로 사용 가능
			Unit.UnitCount += FMath::Pow(3.f, static_cast<float>(UnitLevel) - 1.f);
		}
	}
}

FPCShopUnitData& UPCShopManager::SelectRandomUnitByCost(int32 UnitCost)
{
	// 상점에 남은 해당 코스트 모든 유닛 후보로 추가
	auto& Candidates = GetShopUnitDataListByCost(UnitCost);
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
		DummyData.UnitName = "Dummy";
		return DummyData;
	}

	// 누적합 범위에 따라 기물 선택
	int32 SelectedUnit = 0;
	WeightedRandomSelect<int32>(UnitCounts, 0, TotalUnitCount - 1, SelectedUnit);
	Candidates[SelectedUnit].UnitCount -= 1;
	
	return Candidates[SelectedUnit];
}

void UPCShopManager::ReturnUnitToShopByTag(FGameplayTag UnitTag)
{
	auto UnitCost = GetUnitCostByTag(UnitTag);
	if (UnitCost != 0)
	{
		auto& UnitDataList = GetShopUnitDataListByCost(UnitCost);
		for (auto& Unit : UnitDataList)
		{
			if (Unit.Tag == UnitTag)
			{
				Unit.UnitCount += 1;
			}
		}
	}
}

void UPCShopManager::ReturnUnitsToShopByCarousel(const TArray<FGameplayTag>& UnitTags)
{
	for (auto UnitTag : UnitTags)
	{
		ReturnUnitToShopByTag(UnitTag);
	}
}

void UPCShopManager::ReturnUnitsToShopBySlotUpdate(const TArray<FPCShopUnitData>& OldSlots, const TSet<int32>& PurchasedSlots)
{
	// 구매하지 않은 유닛 상점에 기물 반환
	for (int32 i = 0; i < OldSlots.Num(); ++i)
	{
		if (PurchasedSlots.Contains(i)) continue;

		const auto& OldSlot = OldSlots[i];
		ReturnUnitToShopByTag(OldSlot.Tag);
	}
}

TArray<FGameplayTag> UPCShopManager::GetCarouselUnitTags(int32 Round)
{
	TArray<FGameplayTag> ReturnTags = {};
	switch (Round)
	{
	case 1:
		ReturnTags.Append(GetCarouselRandomUnitTagsByCost(1, 9));
		break;
	case 2:
		ReturnTags.Append(GetCarouselRandomUnitTagsByCost(1, 1));
		ReturnTags.Append(GetCarouselRandomUnitTagsByCost(2, 4));
		ReturnTags.Append(GetCarouselRandomUnitTagsByCost(3, 4));
		break;
	case 3:
		ReturnTags.Append(GetCarouselRandomUnitTagsByCost(1, 1));
		ReturnTags.Append(GetCarouselRandomUnitTagsByCost(2, 2));
		ReturnTags.Append(GetCarouselRandomUnitTagsByCost(3, 3));
		ReturnTags.Append(GetCarouselRandomUnitTagsByCost(4, 3));
		break;
	default:
		ReturnTags.Append(GetCarouselRandomUnitTagsByCost(1, 1));
		ReturnTags.Append(GetCarouselRandomUnitTagsByCost(2, 2));
		ReturnTags.Append(GetCarouselRandomUnitTagsByCost(3, 2));
		ReturnTags.Append(GetCarouselRandomUnitTagsByCost(4, 2));
		ReturnTags.Append(GetCarouselRandomUnitTagsByCost(5, 2));
		break;
	}

	return ReturnTags;
}

TArray<FGameplayTag> UPCShopManager::GetCarouselRandomUnitTagsByCost(int32 UnitCost, int32 CarouselCount)
{
	TArray<FGameplayTag> ReturnTags = {};
	
	for (int i = 0; i < CarouselCount; ++i)
	{
		auto& Unit = SelectRandomUnitByCost(UnitCost);
		// 해당 코스트에 아무 기물도 안남았으면 Add 안함
		if (Unit.UnitName != "Dummy")
		{
			ReturnTags.Add(Unit.Tag);
		}
	}

	return ReturnTags;
}

const TArray<FPCShopUnitData>& UPCShopManager::GetShopUnitDataList()
{
	return ShopUnitDataList;
}

const TArray<FPCShopUnitProbabilityData>& UPCShopManager::GetShopUnitProbabilityDataList()
{
	return ShopUnitProbabilityDataList;
}

const TMap<TPair<int32, int32>, int32>& UPCShopManager::GetShopUnitSellingPriceDataMap()
{
	return ShopUnitSellingPriceDataMap;
}

TArray<float> UPCShopManager::GetCostProbabilities(int32 PlayerLevel)
{
	// 플레이어 레벨에 따라 DataList 탐색
	const auto& ProbData = ShopUnitProbabilityDataList.FindByPredicate(
		[PlayerLevel](const FPCShopUnitProbabilityData& Data)
		{
			return Data.PlayerLevel == PlayerLevel;
		});
	
	TArray<float> CostProbabilities = {
		ProbData->Probability_Cost1,
		ProbData->Probability_Cost2,
		ProbData->Probability_Cost3,
		ProbData->Probability_Cost4,
		ProbData->Probability_Cost5
	};

	return CostProbabilities;
}

TArray<FPCShopUnitData>& UPCShopManager::GetShopUnitDataListByCost(int32 UnitCost)
{
	switch (UnitCost)
	{
	case 1:
		return ShopUnitDataList_Cost1;
	case 2:
		return ShopUnitDataList_Cost2;
	case 3:
		return ShopUnitDataList_Cost3;
	case 4:
		return ShopUnitDataList_Cost4;
	case 5:
		return ShopUnitDataList_Cost5;
	default:
		break;
	}

	// Cost값이 1-5의 값이 아니면, 전체 배열 반환
	return ShopUnitDataList;
}

int32 UPCShopManager::GetUnitCostByTag(FGameplayTag UnitTag)
{
	for (const FPCShopUnitData& UnitData : ShopUnitDataList)
	{
		if (UnitData.Tag == UnitTag)
		{
			return UnitData.UnitCost;
		}
	}

	return 0;
}

int32 UPCShopManager::GetSellingPrice(const TPair<int32, int32>& UnitLevelCostData)
{
	if (const int32* Price = ShopUnitSellingPriceDataMap.Find(UnitLevelCostData))
	{
		return *Price;
	}

	return 0;
}
