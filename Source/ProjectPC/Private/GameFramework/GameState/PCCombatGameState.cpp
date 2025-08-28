// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFramework/GameState/PCCombatGameState.h"

#include "Net/UnrealNetwork.h"
#include "Shop/PCShopManager.h"


APCCombatGameState::APCCombatGameState()
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

	ShopManager->UpdateShopSlots();
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

TArray<float> APCCombatGameState::GetCostProbabilities()
{
	uint8 PlayerLevel = 10;
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
