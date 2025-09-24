// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFramework/GameState/PCCombatGameState.h"

#include "EngineUtils.h"
#include "GameFramework/HelpActor/PCCombatBoard.h"
#include "GameFramework/HelpActor/PCCombatManager.h"
#include "GameFramework/WorldSubsystem/PCProjectilePoolSubsystem.h"
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

	if (auto* ProjectilePoolSubsystem = GetWorld()->GetSubsystem<UPCProjectilePoolSubsystem>())
	{
		if (ProjectilePoolData)
		{
			ProjectilePoolSubsystem->InitializeProjectilePoolData(ProjectilePoolData->ProjectilePoolData);
		}
	}

	if (LevelMaxXPDataTable)
	{
		LoadDataTable<FPCLevelMaxXPData>(LevelMaxXPDataTable, LevelMaxXPDataList, TEXT("Loading Level MaxXP Data"));
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

	bBoardMappingComplete = true;
}

APCCombatBoard* APCCombatGameState::GetBoardBySeat(int32 PlayerSeatIndex) const
{
	return SeatToBoard.IsValidIndex(PlayerSeatIndex) ? SeatToBoard[PlayerSeatIndex] : nullptr;
}

void APCCombatGameState::SetStageRunTime(const FStageRuntimeState& NewState)
{
	if (!HasAuthority())
		return;

	StageRuntimeState = NewState;

	OnRep_StageRunTime();
}

UPCTileManager* APCCombatGameState::GetBattleTileManagerForSeat(int32 SeatIdx) const
{
	UWorld* World = GetWorld();
	if (!World) return nullptr;

	APCCombatManager* CombatManager = nullptr;
	for (TActorIterator<APCCombatManager> It(World); It; ++It)
	{
		CombatManager = *It;
		break;
	}
	if (!CombatManager) return nullptr;

	const int32 PairIdx = CombatManager->FindRunningPairIndexBySeat(SeatIdx);
	if (PairIdx == INDEX_NONE)
	{
		return nullptr;
	}

	if (auto HostBoard = CombatManager->Pairs[PairIdx].Host.Get())
		return HostBoard->TileManager;
	return nullptr;
}

APCCombatBoard* APCCombatGameState::GetBattleBoardForSeat(int32 SeatIdx) const
{
	UWorld* World = GetWorld();
	if (!World) return nullptr;

	APCCombatManager* CombatManager = nullptr;
	for (TActorIterator<APCCombatManager> It(World); It; ++It)
	{
		CombatManager = *It;
		break;
	}
	if (!CombatManager) return nullptr;

	const int32 PairIdx = CombatManager->FindRunningPairIndexBySeat(SeatIdx);
	if (PairIdx == INDEX_NONE)
	{
		return nullptr;
	}

	if (auto HostBoard = CombatManager->Pairs[PairIdx].Host.Get())
		return HostBoard;
	return nullptr;
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

	DOREPLIFETIME_CONDITION_NOTIFY(APCCombatGameState, GameStateTag, COND_None, REPNOTIFY_OnChanged);
	DOREPLIFETIME(APCCombatGameState, StageRuntimeState);
	DOREPLIFETIME(APCCombatGameState, SeatToBoard);
	DOREPLIFETIME(APCCombatGameState, bBoardMappingComplete);
	
}

int32 APCCombatGameState::GetMaxXP(int32 PlayerLevel) const
{
	if (PlayerLevel <= 0 || LevelMaxXPDataList.IsEmpty())
	{
		return 0;
	}
	
	return LevelMaxXPDataList[PlayerLevel - 1].MaxXP;
}

// Game State Tag 변경은 서버에서만 실행
void APCCombatGameState::SetGameStateTag(const FGameplayTag& InGameStateTag)
{
	if (HasAuthority() && GameStateTag != InGameStateTag)
	{
		GameStateTag = InGameStateTag;
		OnGameStateTagChanged.Broadcast(GameStateTag);
	}
}

// 클라에서 Game State 변경 알림 받기 위해 구현
void APCCombatGameState::OnRep_GameStateTag()
{
	OnGameStateTagChanged.Broadcast(GameStateTag);
}

void APCCombatGameState::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
	TagContainer.AddTag(GameStateTag);
}

bool APCCombatGameState::HasMatchingGameplayTag(FGameplayTag TagToCheck) const
{
	return GameStateTag.MatchesTag(TagToCheck);
}

bool APCCombatGameState::HasAllMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const
{
	return GameStateTag.MatchesAny(TagContainer);
}

bool APCCombatGameState::HasAnyMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const
{
	return GameStateTag.MatchesAny(TagContainer);
}
