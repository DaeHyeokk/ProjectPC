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
#include "GameFramework/HelpActor/PCPlayerBoard.h"
#include "GameFramework/HelpActor/Component/PCTileManager.h"


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
	
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		if (auto GS = Cast<APCCombatGameState>(GetOwner()))
		{
			GS->OnGameStateTagChanged.AddUObject(this, &UPCShopManager::OnGameStateChanged);
		}
	}
}

void UPCShopManager::OnGameStateChanged(FGameplayTag NewTag)
{
	if (!GetOwner()) return;
	
	auto GS = Cast<APCCombatGameState>(GetOwner());
	if (!GS) return;
		
	if (NewTag == GameStateTags::Game_State_NonCombat)
	{
		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			if (*It == nullptr) continue;
			
			if (auto PS = (*It)->GetPlayerState<APCPlayerState>())
			{
				auto Board = PS->GetPlayerBoard();
				if (!Board) continue;

				auto BenchUnitTags = Board->GetAllBenchUnitTag();
				for (int i = 0; i < BenchUnitTags.Num(); ++i)
				{
					UnitLevelUp(PS, BenchUnitTags[i], 0);
				}
			}
		}
	}
}

void UPCShopManager::UpdateShopSlots(APCPlayerState* TargetPlayer)
{
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;
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

void UPCShopManager::BuyUnit(APCPlayerState* TargetPlayer, int32 SlotIndex, FGameplayTag UnitTag)
{
	if (!GetOwner()) return;
	if (!TargetPlayer) return;

	auto GS = Cast<APCCombatGameState>(GetOwner());
	if (!GS) return;
	
	auto PlayerBoard = TargetPlayer->GetPlayerBoard();
	
	auto Unit = GetWorld()->GetSubsystem<UPCUnitSpawnSubsystem>()->SpawnUnitByTag(UnitTag, TargetPlayer->SeatIndex);

	auto BenchIndex = PlayerBoard->GetFirstEmptyBenchIndex();

	if (BenchIndex != INDEX_NONE)
	{
		PlayerBoard->PlaceUnitOnBench(BenchIndex, Unit);
	}
		
	UnitLevelUp(TargetPlayer, UnitTag, 0);
	TargetPlayer->PurchasedSlots.Add(SlotIndex);
}

TMap<int32, int32> UPCShopManager::GetLevelUpUnitMap(const APCPlayerState* TargetPlayer, FGameplayTag UnitTag, int32 ShopAddUnitCount) const
{
	TMap<int32, int32> UnitCountByLevelMap;
	UnitCountByLevelMap.Add({1,0});
	UnitCountByLevelMap.Add({2,0});
	UnitCountByLevelMap.Add({3,0});

	if (!GetOwner()) return UnitCountByLevelMap;
	if (!TargetPlayer) return UnitCountByLevelMap;

	auto GS = Cast<APCCombatGameState>(GetOwner());
	if (!GS) return UnitCountByLevelMap;

	auto PlayerBoard = TargetPlayer->PlayerBoard;
	if (!PlayerBoard) return UnitCountByLevelMap;

	TArray<APCBaseUnitCharacter*> UnitList;
	auto CurrentGameStateTag = GS->GetGameStateTag();

	if (CurrentGameStateTag == GameStateTags::Game_State_NonCombat)
	{
		UnitList = PlayerBoard->GetAllUnitByTag(UnitTag, TargetPlayer->SeatIndex);
	}
	else
	{
		UnitList = PlayerBoard->GetBenchUnitByTag(UnitTag, TargetPlayer->SeatIndex);
	}
	
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
	if (!TargetPlayer) return 0;

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

void UPCShopManager::UnitLevelUp(const APCPlayerState* TargetPlayer, FGameplayTag UnitTag, int32 ShopAddUnitCount)
{
	if (!GetOwner()) return;
	if (!TargetPlayer) return;
	
	auto GS = Cast<APCCombatGameState>(GetOwner());
	if (!GS) return;

	auto PlayerBoard = TargetPlayer->PlayerBoard;
	if (!PlayerBoard) return;

	TArray<APCBaseUnitCharacter*> UnitList;
	auto CurrentGameStateTag = GS->GetGameStateTag();

	if (CurrentGameStateTag == GameStateTags::Game_State_NonCombat)
	{
		UnitList = PlayerBoard->GetAllUnitByTag(UnitTag, TargetPlayer->SeatIndex);
	}
	else
	{
		UnitList = PlayerBoard->GetBenchUnitByTag(UnitTag, TargetPlayer->SeatIndex);
	}

	auto AddShopUnitCountMap = GetLevelUpUnitMap(TargetPlayer, UnitTag, ShopAddUnitCount);
	
	AddShopUnitCountMap.KeySort([](const int32 A, const int32 B){ return A < B; });

	TArray<int32> LevelUp;
	
	// 레벨이 낮은 것부터 합치고 UnitCountByLevelMap 업데이트
	for (auto& Pair : AddShopUnitCountMap)
	{
		auto UnitLevel = Pair.Key;
		auto& UnitCount = Pair.Value;
		
		if (UnitCount >= 3)
		{
			UnitCount -= 3;
			AddShopUnitCountMap.FindOrAdd(UnitLevel + 1)++;
			LevelUp.Add(UnitLevel);
		}
	}

	for (int32 UnitLevel : LevelUp)
	{
		TArray<APCHeroUnitCharacter*> HeroUnitList;
		for (auto Unit : UnitList)
		{
			if (auto* Hero = Cast<APCHeroUnitCharacter>(Unit))
			{
				if (IsValid(Hero) && !Hero->IsActorBeingDestroyed() && Hero->GetUnitLevel() == UnitLevel)
				{
					HeroUnitList.Add(Hero);
				}
			}
		}

		APCHeroUnitCharacter* LevelUpUnit = HeroUnitList[0];
		LevelUpUnit->LevelUp();

		int32 RemoveCount = FMath::Min(2, HeroUnitList.Num() - 1);
		for (int32 i = 1; i <= RemoveCount; ++i)
		{
			PlayerBoard->RemoveFromBoard(HeroUnitList[i]);
			HeroUnitList[i]->Destroy();
		}
	}
}

void UPCShopManager::SellUnit(FGameplayTag UnitTag, int32 UnitLevel)
{
	if (!GetOwner()) return;
	
	auto GS = Cast<APCCombatGameState>(GetOwner());
	if (!GS) return;

	auto UnitCost = GetUnitCostByTag(UnitTag);

	for (auto& Unit : GetShopUnitDataListByCost(UnitCost))
	{
		if (UnitTag == Unit.UnitTag)
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
			if (Unit.UnitTag == UnitTag)
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
		ReturnUnitToShopByTag(OldSlot.UnitTag);
	}
}

TArray<FGameplayTag> UPCShopManager::GetCarouselUnitTags(int32 Round)
{
	TArray<FGameplayTag> ReturnTags = {};
	switch (Round)
	{
	case 1:
		ReturnTags.Append(GetCarouselRandomUnitTagsByCost(1, 8));
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
			ReturnTags.Add(Unit.UnitTag);
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
		if (UnitData.UnitTag == UnitTag)
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
