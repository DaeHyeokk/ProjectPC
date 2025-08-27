// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFramework/GameState/PCCombatGameState.h"

#include "Net/UnrealNetwork.h"
#include "Shop/PCShopManager.h"


APCCombatGameState::APCCombatGameState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ShopManager = CreateDefaultSubobject<UPCShopManager>(TEXT("ShopManager"));
}

void APCCombatGameState::BeginPlay()
{
	Super::BeginPlay();

	if (ShopUnitDataTable && ShopUnitProbabilityDataTable && ShopUnitSellingPriceDataTable)
	{
		LoadDataTable<FPCShopUnitData>(ShopUnitDataTable, ShopUnitDataList, TEXT("Loading Shop Unit Data"));
		LoadDataTable<FPCShopUnitProbabilityData>(ShopUnitProbabilityDataTable, ShopUnitProbabilityDataList, TEXT("Loading Shop Unit Probability Data"));
		LoadDataTableToMap<FPCShopUnitSellingPriceData>(ShopUnitSellingPriceDataTable, ShopUnitSellingPriceDataMap, TEXT("Loading Shop Unit Selling Price Data"));
	}

	// UE_LOG(LogTemp, Warning, TEXT("===== ShopUnitDataList ====="));
	// for (const auto& Data : ShopUnitDataList)
	// {
	// 	UE_LOG(LogTemp, Warning, TEXT("UnitName: %s, UnitCost: %d"), *Data.UnitName.ToString(), Data.UnitCost);
	// }
	//
	// UE_LOG(LogTemp, Warning, TEXT("===== ShopUnitProbabilityDataList ====="));
	// for (const auto& Data : ShopUnitProbabilityDataList)
	// {
	// 	UE_LOG(LogTemp, Warning, TEXT("PlayerLevel: %d, Probability_Cost1: %f, Probability_Cost2: %f, Probability_Cost3: %f, Probability_Cost4: %f, Probability_Cost5: %f"), Data.PlayerLevel, Data.Probability_Cost1, Data.Probability_Cost2, Data.Probability_Cost3, Data.Probability_Cost4, Data.Probability_Cost5);
	// }
	//
	// UE_LOG(LogTemp, Warning, TEXT("===== ShopUnitSellingPriceDataMap ====="));
	// for (const auto& Elem : ShopUnitSellingPriceDataMap)
	// {
	// 	const TPair<uint8, uint8>& Key = Elem.Key;
	// 	uint8 Value = Elem.Value;
	// 	UE_LOG(LogTemp, Warning, TEXT("Cost: %d, Level: %d -> SellingPrice: %d"), Key.Key, Key.Value, Value);
	// }

	UpdateShopSlots();
}

FString APCCombatGameState::GetStageRoundLabel() const
{
	return FString::Printf(TEXT("%d-%d"), StageIdx+1, RoundIdx+1);
}

float APCCombatGameState::GetRemainingSeconds() const
{
	return FMath::Max(0.f, StageEndTime_Server - GetServerWorldTimeSeconds());
}

void APCCombatGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(APCCombatGameState, FloatIndex);
	DOREPLIFETIME(APCCombatGameState, StageIdx);
	DOREPLIFETIME(APCCombatGameState, RoundIdx);
	DOREPLIFETIME(APCCombatGameState, StepIdxInRound);
	DOREPLIFETIME(APCCombatGameState, CurrentStage);
	DOREPLIFETIME(APCCombatGameState, StageDuration);
	DOREPLIFETIME(APCCombatGameState, StageEndTime_Server);
}

TArray<FPCShopUnitData>& APCCombatGameState::GetShopUnitDataList()
{
	return ShopUnitDataList;
}

const TArray<FPCShopUnitProbabilityData>& APCCombatGameState::GetShopUnitProbabilityDataList()
{
	return ShopUnitProbabilityDataList;
}

const TMap<TPair<uint8, uint8>, uint8>& APCCombatGameState::GetShopUnitSellingPriceDataMap()
{
	return ShopUnitSellingPriceDataMap;
}

void APCCombatGameState::UpdateShopSlots()
{
	if (!ShopManager) return;

	uint8 NumSlots = 5;
	uint8 PlayerLevel = 10;
	TArray<FPCShopUnitData> NewShopSlots;

	// 플레이어 레벨에 따른 확률 정보
	auto ProbData = ShopUnitProbabilityDataList.FindByPredicate(
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

	for (uint8 i = 0; i < NumSlots; ++i)
	{
		float RandomValue = FMath::RandRange(0.f, 1.f);
		uint8 SelectedCost = 1;
		float Accum = 0.f;

		// 누적합 범위에 따라 코스트 선택
		for (uint8 CostIndex = 0; CostIndex < CostProbabilities.Num(); ++CostIndex)
		{
			Accum += CostProbabilities[CostIndex];
			if (RandomValue <= Accum)
			{
				SelectedCost = CostIndex + 1;
				break;
			}
		}
		
		TArray<FPCShopUnitData> Candidates;
		uint16 TotalUnitCount = 0;
		for (const auto& Unit : ShopUnitDataList)
		{
			if (Unit.UnitCost == SelectedCost && Unit.UnitCount > 0)
			{
				Candidates.Add(Unit);
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

		uint16 RandomIndex = FMath::RandRange(0, TotalUnitCount - 1);
		uint16 AccumCount = 0;

		// 누적합 범위에 따라 기물 선택
		for (const auto& Unit : Candidates)
		{
			AccumCount += Unit.UnitCount;
			if (RandomIndex < AccumCount)
			{
				NewShopSlots.Add(Unit);
				break;
			}
		}
	}

	ShopManager->SetupShopSlots(NewShopSlots);
}
