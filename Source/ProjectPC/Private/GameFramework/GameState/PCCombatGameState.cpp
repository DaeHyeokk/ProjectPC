// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFramework/GameState/PCCombatGameState.h"

#include "GameFramework/HelpActor/PCCombatBoard.h"
#include "GameFramework/WorldSubsystem/PCUnitSpawnSubsystem.h"
#include "Net/UnrealNetwork.h"
#include "Shop/PCShopManager.h"


APCCombatGameState::APCCombatGameState()
{
	ShopManager = CreateDefaultSubobject<UPCShopManager>(TEXT("ShopManager"));
}

void APCCombatGameState::BeginPlay()
{
	Super::BeginPlay();
	
	if (auto* UnitSpawnSubsystem = GetWorld()->GetSubsystem<UPCUnitSpawnSubsystem>())
	{
		UnitSpawnSubsystem->InitializeUnitSpawnConfig(SpawnConfig);
	}
	
	if (ShopUnitDataTable && ShopUnitProbabilityDataTable && ShopUnitSellingPriceDataTable)
	{
		LoadDataTable<FPCShopUnitData>(ShopUnitDataTable, ShopUnitDataList, TEXT("Loading Shop Unit Data"));
		LoadDataTable<FPCShopUnitProbabilityData>(ShopUnitProbabilityDataTable, ShopUnitProbabilityDataList, TEXT("Loading Shop Unit Probability Data"));
		LoadDataTable<FPCLevelMaxXPData>(LevelMaxXPDataTable, LevelMaxXPDataList, TEXT("Loading Level MaxXP Data"));
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

void APCCombatGameState::BuildSeatToBoardMap(const TArray<APCCombatBoard*>& Boards)
{
	int32 MaxSeat = 0;
	for (const APCCombatBoard* CombatBoard : Boards)
	{
		if (CombatBoard)
		{
			MaxSeat = FMath::Max(MaxSeat, CombatBoard->BoardSeatIndex);
		}
	}
	SeatToBoard.SetNum(MaxSeat+1);
	for (APCCombatBoard* CombatBoard : Boards)
	{
		if (CombatBoard && CombatBoard->BoardSeatIndex >= 0)
		{
			SeatToBoard[CombatBoard->BoardSeatIndex] = CombatBoard;
		}
	}
}

APCCombatBoard* APCCombatGameState::GetBoardBySeat(int32 PlayerSeatIndex) const
{
	return SeatToBoard.IsValidIndex(PlayerSeatIndex) ? SeatToBoard[PlayerSeatIndex].Get() : nullptr;
}

void APCCombatGameState::SetStageRunTime(const FStageRuntimeState& NewState)
{
	if (!HasAuthority())
		return;

	StageRuntimeState = NewState;

	OnRep_StageRunTime();
}

float APCCombatGameState::GetStageRemainingSeconds() const
{
	const float Now = GetServerWorldTimeSeconds();
	return FMath::Max(0.f, StageRuntimeState.ServerEndTime - Now);
}

float APCCombatGameState::GetStageProgress() const
{
	const float Now = GetServerWorldTimeSeconds();
	const float Elapsed = Now - StageRuntimeState.ServerStartTime;
	return (StageRuntimeState.Duration > 0.f) ? FMath::Clamp(Elapsed / StageRuntimeState.Duration, 0.f, 1.f) : 1.f;
}

FString APCCombatGameState::GetStageLabelString() const
{
	return FString::Printf(TEXT("%d-%d"), StageRuntimeState.StageIdx, StageRuntimeState.RoundIdx);
}

EPCStageType APCCombatGameState::GetCurrentStageType() const
{
	return StageRuntimeState.Stage;
}

void APCCombatGameState::OnRep_StageRunTime()
{
	OnStageRuntimeChanged.Broadcast();
}


void APCCombatGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APCCombatGameState, GameStateTag);
	DOREPLIFETIME(APCCombatGameState, StageRuntimeState);
	
}

const TArray<FPCShopUnitData>& APCCombatGameState::GetShopUnitDataList()
{
	return ShopUnitDataList;
}

const TArray<FPCShopUnitProbabilityData>& APCCombatGameState::GetShopUnitProbabilityDataList()
{
	return ShopUnitProbabilityDataList;
}

const TMap<TPair<int32, int32>, int32>& APCCombatGameState::GetShopUnitSellingPriceDataMap()
{
	return ShopUnitSellingPriceDataMap;
}

TArray<float> APCCombatGameState::GetCostProbabilities(int32 PlayerLevel)
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

TArray<FPCShopUnitData>& APCCombatGameState::GetShopUnitDataListByCost(int32 Cost)
{
	switch (Cost)
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

const int32 APCCombatGameState::GetMaxXP(int32 PlayerLevel) const
{
	return LevelMaxXPDataList[PlayerLevel - 1].MaxXP;
}

// Game State Tag 변경은 서버에서만 실행
void APCCombatGameState::SetGameStateTag(const FGameplayTag& InGameStateTag)
{
	if (HasAuthority() && GameStateTag != InGameStateTag)
	{
		GameStateTag = InGameStateTag;
		OnRep_GameStateTag();	// 서버에서도 값이 변경됨을 알려야하므로 직접 호출
	}
}

// 클라는 자동으로 호출
void APCCombatGameState::OnRep_GameStateTag() const
{
	OnGameStateChanged.Broadcast(GameStateTag);
}
