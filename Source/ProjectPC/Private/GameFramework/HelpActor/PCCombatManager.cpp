


#include "GameFramework/HelpActor/PCCombatManager.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "EngineUtils.h"
#include "IContentBrowserSingleton.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "Character/Player/PCPlayerCharacter.h"
#include "Character/Unit/PCHeroUnitCharacter.h"
#include "Controller/Player/PCCombatPlayerController.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/GameState/PCCombatGameState.h"
#include "GameFramework/HelpActor/PCCombatBoard.h"
#include "GameFramework/HelpActor/Component/PCTileManager.h"
#include "GameFramework/HelpActor/DataTable/StageData.h"
#include "GameFramework/PlayerState/PCPlayerState.h"
#include "GameFramework/WorldSubsystem/PCUnitSpawnSubsystem.h"


namespace
{
	// (Y,X) 로 받았으니 FIntPoint.X=Y, FIntPoint.Y=X 로 저장
	static bool BuildCreepPoints(int32 StageOne, int32 RoundOne, TArray<FIntPoint>& Out)
	{
		Out.Reset();

		switch (StageOne)
		{
		case 1:
			switch (RoundOne)
			{
		//case 2: Out = { FIntPoint(5,2), FIntPoint(5,4) }; return true;
		case 2: Out = { FIntPoint(5,2), FIntPoint(5,4), FIntPoint(6,1) }; return true;
		case 3: Out = { FIntPoint(5,2), FIntPoint(5,4), FIntPoint(6,1) }; return true;
		case 4: Out = { FIntPoint(5,2), FIntPoint(5,4), FIntPoint(6,1), FIntPoint(6,4) }; return true;
		default: break;
			}
			break;

		case 2:
			switch (RoundOne)
			{
		case 7: Out = { FIntPoint(4,0), FIntPoint(4,5), FIntPoint(6,1) }; return true;
		default: break;
			}
			break;

		case 3:
			switch (RoundOne)
			{
		case 7: Out = { FIntPoint(5,3), FIntPoint(7,1), FIntPoint(7,2), FIntPoint(7,4), FIntPoint(7,5) }; return true;
		default: break;
			}
			break;

		case 4:
			switch (RoundOne)
			{
		case 7: Out = { FIntPoint(5,1), FIntPoint(5,5), FIntPoint(6,1), FIntPoint(6,4), FIntPoint(7,3) }; return true;
		default: break;
			}
			break;

		case 5:
			switch (RoundOne)
			{
		case 7: Out = { FIntPoint(5,3) }; return true;
		default: break;
			}
			break;

		case 6:
			switch (RoundOne)
			{
		case 7: Out = { FIntPoint(5,3) }; return true;
		default: break;
			}
			break;

		default: break;
		}
		return false;
	}
}


APCCombatManager::APCCombatManager()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
}

void APCCombatManager::BuildRandomPairs()
{
	if (!IsAuthority())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
		return;
	APCCombatGameState* PCGameState = World->GetGameState<APCCombatGameState>();
	if (!PCGameState)
		return;

	TArray<APCPlayerState*> Players;
	if (AGameStateBase* GameState = World->GetGameState())
	{
		for (APlayerState* PlayerState : GameState->PlayerArray)
		{
			if (APCPlayerState* PCPlayerState = Cast<APCPlayerState>(PlayerState))
			{
				if (PCPlayerState->SeatIndex >= 0)
				{
					Players.Add(PCPlayerState);
				}
			}
		}
	}

	Pairs.Reset();
	UnitToPairIndex.Empty();

	if (Players.IsEmpty())
	{
		return;
	}

	// 셔플
	int32 Seed = bReseedEveryRound ? (RandomSeed ^ FMath::Rand()) : RandomSeed;
	FRandomStream RNG(Seed);
	for (int32 i = Players.Num() - 1; i >0; --i)
	{
		Players.Swap(i, RNG.RandRange(0,i));
	}
	
	for (int32 i = 0; i < Players.Num(); i+=2)
	{
		APCCombatBoard* HostBoard = PCGameState->GetBoardBySeat(Players[i]->SeatIndex);
		APCCombatBoard* GuestBoard = (i+1 < Players.Num()) ? PCGameState->GetBoardBySeat(Players[i+1]->SeatIndex) : nullptr;

		FCombatManager_Pair Pair;
		Pair.Host = HostBoard;
		Pair.Guest = GuestBoard;
		Pair.GuestSnapShot.Reset();
		Pair.MovedUnits.Reset();
		Pair.HostAlive = 0;
		Pair.GuestAlive = 0;
		Pair.bRunning = false;
		Pair.DeadUnits.Reset();
		Pairs.Add(Pair);
	}
}

void APCCombatManager::StartAllBattle()
{
	if (!IsAuthority())
		return;

	if (UPCUnitSpawnSubsystem* SpawnSubsystem = GetWorld()->GetSubsystem<UPCUnitSpawnSubsystem>())
	{
		SpawnSubsystem->OnUnitSpawned.AddUObject(this, &APCCombatManager::OnUnitSpawnedDuringBattle);
	}

	for (int32 PairIndex = 0; PairIndex < Pairs.Num(); ++PairIndex)
	{
		auto& Pair = Pairs[PairIndex];
		APCCombatBoard* Host  = Pair.Host.Get();
		APCCombatBoard* Guest = Pair.Guest.Get();
		if (!Host || !Guest) continue;

		UPCTileManager* HostTM  = Host->TileManager;
		UPCTileManager* GuestTM = Guest->TileManager;
		if (!HostTM || !GuestTM) continue;

		// 양쪽 스냅샷 저장
		TakeSnapshot(Host, Pair.HostSnapShot);
		TakeSnapshot(Guest, Pair.GuestSnapShot);
		
		Pair.MovedUnits.Reset();
		for (const auto& Field : Pair.GuestSnapShot.Field)
		{
			if (Field.Unit.IsValid())
			{
				Pair.MovedUnits.Add(Field.Unit);
			}
		}

		for (const auto& Bench : Pair.GuestSnapShot.Bench)
		{
			if (Bench.Unit.IsValid())
			{
				Pair.MovedUnits.Add(Bench.Unit);
			}
		}

		GuestTM->MoveUnitsMirroredTo(HostTM, bMirrorRows, bMirrorCols, bIncludeBench);

		CountAliveOnHostBoardForPair(PairIndex);

		BindUnitOnBoardForPair(PairIndex);

		Pair.NewUnitDuringBattle.Reset();
		Pair.bRunning = true;
	}
}

void APCCombatManager::FinishAllBattle()
{
	if (!IsAuthority())
		return;

	if (UPCUnitSpawnSubsystem* SpawnSubsystem = GetWorld()->GetSubsystem<UPCUnitSpawnSubsystem>())
	{
		SpawnSubsystem->OnUnitSpawned.RemoveAll(this);
	}

	for (int32 PairIndex = 0; PairIndex < Pairs.Num(); ++PairIndex)
	{
		auto& Pair = Pairs[PairIndex];
		APCCombatBoard* Host = Pair.Host.Get();
		APCCombatBoard* Guest = Pair.Guest.Get();
		if (!Host)
			continue;

		UPCTileManager* HostTM = Host->TileManager;
		UPCTileManager* GuestTM = Guest->TileManager;

		// 넘어온 게스트 유닛 제거
		if (HostTM)
		{
			for (const auto& WU : Pair.MovedUnits)
			{
				if (APCBaseUnitCharacter* Unit = WU.Get())
				{
					RemoveUnitFromAny(HostTM, Unit);
				}
			}
		}

		// 게스트 스냅샷 원복
		if (Guest && GuestTM)
		{
			RestoreSnapshot(Pair.GuestSnapShot);
		}

		// 호스트 스냅샷 원복
		if (Host && HostTM)
		{
			RestoreSnapshot(Pair.HostSnapShot);
		}
		
		// 전투 도중 새로 생긴 유닛들 "자기 홈 보드 벤치"로 귀환
		if (APCCombatGameState* PCGameState = GetWorld()->GetGameState<APCCombatGameState>())
		{
			for (auto& NewUnit : Pair.NewUnitDuringBattle)
			{
				const int32 Seat = NewUnit.Key;
				APCCombatBoard* HomeBoard = PCGameState->GetBoardBySeat(Seat);
				if (!HomeBoard || !HomeBoard->TileManager)
					continue;

				for (auto& WeakUnit : NewUnit.Value)
				{
					APCBaseUnitCharacter* Unit = WeakUnit.Get();
					if (!IsValid(Unit))
						continue;

					// 현재 위치에서 제거
					if (APCCombatBoard* CurBoard = Unit->GetOnCombatBoard())
					{
						if (CurBoard->TileManager)
						{
							CurBoard->TileManager->RemoveFromBoard(Unit);
						}
					}

					const int32 idx = FindFirstFreeBenchIndex(HomeBoard->TileManager, false);
					if (idx != INDEX_NONE)
					{
						HomeBoard->TileManager->PlaceUnitOnBench(idx, Unit, ETileFacing::Friendly);
					}
				}
			}
		}

		// 언바인드
		UnbindAllForPair(PairIndex);
		Pair.MovedUnits.Reset();
		Pair.NewUnitDuringBattle.Reset();
		Pair.bRunning = false;
	}
}

APCCombatBoard* APCCombatManager::FindBoardBySeatIndex(UWorld* World, int32 SeatIndex)
{
	if (!World)
		return nullptr;
	for (TActorIterator<APCCombatBoard> It(World); It; ++It)
	{
		if (It->BoardSeatIndex == SeatIndex)
		{
			return *It;
		}
	}
	return nullptr;
}

void APCCombatManager::TakeSnapshot(APCCombatBoard* Board, FCombatManager_BoardSnapShot& BoardSnapShot)
{
	BoardSnapShot.Reset();
	BoardSnapShot.CombatBoard = Board;
	BoardSnapShot.Tile = Board ? Board->TileManager : nullptr;
	UPCTileManager* TileManager = BoardSnapShot.Tile.Get();
	if (!TileManager)
		return;

	const int32 Rows = TileManager->Rows;
	const int32 Cols = TileManager->Cols;

	// 필드
	for (int32 r = 0; r < Rows; ++r)
	{
		for (int32 c = 0; c < Cols; ++c)
		{
			if (APCBaseUnitCharacter* InUnit = TileManager->GetFieldUnit(c,r))
			{
				FCombatManager_FieldSlot FieldSlot;
				FieldSlot.Col = c;
				FieldSlot.Row = r;
				FieldSlot.Unit = InUnit;
				BoardSnapShot.Field.Add(FieldSlot);
			}
		}
	}

	// 벤치
	for (int32 i = 0; i < TileManager->Bench.Num(); ++i)
	{
		if (APCBaseUnitCharacter* InUnit = TileManager->GetBenchUnit(i))
		{
			FCombatManager_BenchSlot BenchSlot;
			BenchSlot.Index = i;
			BenchSlot.Unit = InUnit;
			BoardSnapShot.Bench.Add(BenchSlot);
		}
	}
}

void APCCombatManager::RestoreSnapshot(const FCombatManager_BoardSnapShot& Snap)
{
	UPCTileManager* TileManager = Snap.Tile.Get();
	if (!TileManager)
		return;

	TileManager->ClearAll();

	for (const auto& FieldSlot : Snap.Field)
	{
		if (APCBaseUnitCharacter* InUnit = FieldSlot.Unit.Get())
		{
			TileManager->PlaceUnitOnField(FieldSlot.Col, FieldSlot.Row, InUnit, ETileFacing::Friendly);
		}
	}

	for (const auto& BenchSlot : Snap.Bench)
	{
		if (APCBaseUnitCharacter* InUnit = BenchSlot.Unit.Get())
		{
			TileManager->PlaceUnitOnBench(BenchSlot.Index, InUnit, ETileFacing::Friendly);
		}
	}
}

bool APCCombatManager::RemoveUnitFromAny(UPCTileManager* TileManager, APCBaseUnitCharacter* Unit)
{
	if (!TileManager || !Unit) return false;

	for (int32 r = 0; r < TileManager->Rows; ++r)
	{
		for (int32 c = 0; c < TileManager->Cols; ++c)
		{
			if (TileManager->GetFieldUnit(c, r) == Unit)
			{
				TileManager->RemoveFromField(c,r,false);
				return true;
			}
		}
	}

	for (int32 i = 0; i < TileManager->Bench.Num(); ++i)
	{
		if (TileManager->GetBenchUnit(i) == Unit)
		{
			TileManager->RemoveFromBench(i,false);
			return true;
		}
	}
	return false;
}

void APCCombatManager::TakeSnapShotPvE(APCCombatBoard* Board, FCombatManager_FieldOnlySnapshot& OutSnap)
{
	OutSnap.Reset();
	if (!IsAuthority() || !Board || !Board->TileManager) return;

	OutSnap.SeatIndex = Board->BoardSeatIndex; // ★ 좌석 저장

	UPCTileManager* TM = Board->TileManager;
	const int32 Rows = TM->Rows;
	const int32 Cols = TM->Cols;

	for (int32 r = 0; r < Rows; ++r)
	{
		for (int32 c = 0; c < Cols; ++c)
		{
			if (APCBaseUnitCharacter* InUnit = TM->GetFieldUnit(c, r))
			{
				FCombatManager_FieldSlot Slot;
				Slot.Col  = c;
				Slot.Row  = r;
				Slot.Unit = InUnit;
				OutSnap.Field.Add(Slot);
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("[PvE Snapshot] Seat=%d FieldSaved=%d"),
		OutSnap.SeatIndex, OutSnap.Field.Num());
}

void APCCombatManager::RestoreSnapShotPvE(FCombatManager_FieldOnlySnapshot& Snap)
{
	if (!IsAuthority())
		return;

	// ★ 스냅샷 내부 포인터 의존 금지: 항상 현재 GameState에서 보드/타일 재획득
	APCCombatGameState* GS = GetWorld() ? GetWorld()->GetGameState<APCCombatGameState>() : nullptr;
	if (!GS)
	{
		UE_LOG(LogTemp, Error, TEXT("[PvE Restore] GameState null"));
		return;
	}

	APCCombatBoard* Board = GS->GetBoardBySeat(Snap.SeatIndex);
	if (!Board || !Board->TileManager)
	{
		UE_LOG(LogTemp, Error, TEXT("[PvE Restore] Board/TM null for Seat=%d"), Snap.SeatIndex);
		return;
	}

	UPCTileManager* TM = Board->TileManager;

	UE_LOG(LogTemp, Log, TEXT("[PvE Restore] Seat=%d TM=%s Rows=%d Cols=%d SavedSlots=%d"),
		Snap.SeatIndex, *TM->GetName(), TM->Rows, TM->Cols, Snap.Field.Num());

	// 1) 현재 필드 비우기 (벤치는 그대로)
	for (int32 r = 0; r < TM->Rows; ++r)
	{
		for (int32 c = 0; c < TM->Cols; ++c)
		{
			if (TM->GetFieldUnit(c, r))
			{
				TM->RemoveFromField(c, r, /*bPreserveUnitBoard=*/false);
			}
		}
	}

	// 2) 스냅샷 필드 복구
	int32 Restored = 0;
	for (const auto& FS : Snap.Field)
	{
		if (APCBaseUnitCharacter* Unit = FS.Unit.Get())
		{
			const bool bOk = TM->PlaceUnitOnField(FS.Col, FS.Row, Unit, ETileFacing::Friendly);
			UE_LOG(LogTemp, Log, TEXT("[PvE Restore] (%d,%d) %s -> %s"),
				   FS.Col, FS.Row, *Unit->GetName(), bOk ? TEXT("OK") : TEXT("FAIL"));
			if (bOk) ++Restored;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[PvE Restore] Snapshot unit null at (%d,%d)"), FS.Col, FS.Row);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("[PvE Restore] Seat=%d Restored %d / %d slots"),
		Snap.SeatIndex, Restored, Snap.Field.Num());
}

// 좌석 기반 조회 헬퍼
APCPlayerState* APCCombatManager::FindPlayerStateBySeat(int32 SeatIndex) const
{
	if (AGameStateBase* GameState = GetWorld() ? GetWorld()->GetGameState() : nullptr)
	{
		for (APlayerState* PlayerState : GameState->PlayerArray)
		{
			if (auto* PCPlayerState = Cast<APCPlayerState>(PlayerState))
			{
				if (PCPlayerState->SeatIndex == SeatIndex)
					return PCPlayerState;
			}
		}
	}
	return nullptr;
}

APCCombatPlayerController* APCCombatManager::FindPlayerController(int32 SeatIndex) const
{
	if (APCPlayerState* PCPlayerState = FindPlayerStateBySeat(SeatIndex))
	{
		return Cast<APCCombatPlayerController>(PCPlayerState->GetOwner());
	}
	return nullptr;
}

APawn* APCCombatManager::FindPawnBySeat(int32 SeatIndex) const
{
	if (APCCombatPlayerController* PCPlayerController = FindPlayerController(SeatIndex))
	{
		return PCPlayerController->GetPawn();
	}
	return nullptr;
}


// 이동 및 카메라 유틸
void APCCombatManager::TeleportPlayerToTransform(APawn* PlayerCharacter, const FTransform& T) const
{
	if (!PlayerCharacter)
		return;
	PlayerCharacter->TeleportTo(T.GetLocation(), T.GetRotation().Rotator(), false, true);
}

void APCCombatManager::FocusCameraToBoard(int32 ViewerSeatIdx, int32 BoardSeatIdx, bool bIsBattle, float Blend)
{
	if (APCCombatPlayerController* CombatController = FindPlayerController(ViewerSeatIdx))
	{
		CombatController->ClientFocusBoardBySeatIndex(BoardSeatIdx, bIsBattle, Blend);
	}
}



void APCCombatManager::TravelPlayersForAllPairs(float Blend)
{
	if (!IsAuthority())
		return;
	for (int i = 0; i < Pairs.Num(); ++i)
	{
		TravelPlayersForPair(i, Blend);
	}
}

void APCCombatManager::ReturnPlayersForAllPairs(float Blend)
{
	if (!IsAuthority())
		return;
	for (int32 i = 0; i < Pairs.Num(); ++i)
	{
		ReturnPlayersForPair(i, Blend);
	}
}

void APCCombatManager::TravelPlayersForPair(int32 PairIndex, float Blend)
{
	if (!IsAuthority() || !Pairs.IsValidIndex(PairIndex))
		return;

	auto& Pair = Pairs[PairIndex];
	APCCombatBoard* Host = Pair.Host.Get();
	APCCombatBoard* Guest = Pair.Guest.Get();

	if (!Host)
		return;

	const int32 HostSeat = Host->BoardSeatIndex;
	const int32 GuestSeat = Guest ? Guest->BoardSeatIndex : INDEX_NONE;

	const FTransform T_Player = Host->GetPlayerSeatTransform();
	const FTransform T_Guest = Host->GetEnemySeatTransform();

	if ( APawn* HostPlayer = FindPawnBySeat(HostSeat))
	{
		TeleportPlayerToTransform(HostPlayer, T_Player);
	}

	if (Guest)
	{
		if (APawn* GuestPlayer = FindPawnBySeat(GuestSeat))
		{
			TeleportPlayerToTransform(GuestPlayer, T_Guest);
		}
	}

	if (auto* HPS = FindPlayerStateBySeat(HostSeat))
	{
		FocusCameraToBoard(HPS->SeatIndex, HostSeat, true, 0);
	}

	if (Guest)
	{
		if (auto* GPS = FindPlayerStateBySeat(GuestSeat))
		{
			FocusCameraToBoard(GPS->SeatIndex, HostSeat, true, 0);
		}
	}
}

void APCCombatManager::ReturnPlayersForPair(int32 PairIndex, float Blend)
{
	if (!IsAuthority() || !Pairs.IsValidIndex(PairIndex))
		return;

	FCombatManager_Pair& Pair = Pairs[PairIndex];
	APCCombatBoard* Host = Pair.Host.Get();
	APCCombatBoard* Guest = Pair.Guest.Get();
	if (!Host) return;

	const int32 HostSeat  = Host->BoardSeatIndex;
	const int32 GuestSeat = Guest ? Guest->BoardSeatIndex : INDEX_NONE;

	if (APawn* PHost = FindPawnBySeat(HostSeat))
	{
		TeleportPlayerToTransform(PHost, Host->GetPlayerSeatTransform());
	}

	if (Guest)
	{
		if (APawn* PGuest = FindPawnBySeat(GuestSeat))
		{
			TeleportPlayerToTransform(PGuest, Guest->GetPlayerSeatTransform());
			PGuest->SetActorRotation(GuestRotation);
		}
	}

	if (APCPlayerState* HostPlayerState = FindPlayerStateBySeat(HostSeat))
	{
		FocusCameraToBoard(HostPlayerState->SeatIndex, HostSeat, false, 0);
	}

	if (Guest)
	{
		if (APCPlayerState* GuestPlayerState = FindPlayerStateBySeat(GuestSeat))
		{
			FocusCameraToBoard(GuestPlayerState->SeatIndex, GuestSeat, false, 0);
		}
	}
}

int32 APCCombatManager::StartPvEBattleForSeat(int32 HostSeatIndex)
{
	if (!IsAuthority()) return INDEX_NONE;

    APCCombatBoard* HostBoard = FindBoardBySeatIndex(GetWorld(), HostSeatIndex);
    if (!HostBoard || !HostBoard->TileManager) return INDEX_NONE;

    int32 StageOne = 1, RoundOne = 2;
    // if (!GetCurrentStageRoundOne(StageOne, RoundOne))
    // {
    //     UE_LOG(LogTemp, Warning, TEXT("[PvE Start] Stage/Round runtime not available."));
    //     return INDEX_NONE;
    // }

    // 좌표 빌드
    TArray<FIntPoint> Points;
    BuildCreepPoints(StageOne, RoundOne, Points);

    // Pair 생성
    const int32 PairIndex = Pairs.AddDefaulted();
    FCombatManager_Pair& Pair = Pairs[PairIndex];
    Pair.ResetRuntime();
    Pair.Host  = HostBoard;
    Pair.Guest = nullptr;
    Pair.bIsPvE = true;

    // ★ 먼저 스냅샷 (필드만)
    TakeSnapShotPvE(HostBoard, Pair.HostFieldSnapshot);

    // 크립 스폰/배치
    for (const FIntPoint& YX : Points)
    {
        if (APCBaseUnitCharacter* Creep = SpawnCreepAt(HostBoard, StageOne, RoundOne, YX))
        {
            if (!UnitToPairIndex.Contains(Creep))
            {
                Creep->OnUnitDied.AddDynamic(this, &APCCombatManager::OnAnyUnitDied);
                UnitToPairIndex.Add(Creep, PairIndex);
            }
            Pair.PvECreeps.Add(Creep);
        }
    }

    // 생존 수
    int32 HostCnt = 0, CreepCnt = 0;
    UPCTileManager* TM = HostBoard->TileManager;
    const int32 CreepTeam = GetCreepTeamIndexForBoard(HostBoard);

    for (int32 r = 0; r < TM->Rows; ++r)
    {
        for (int32 c = 0; c < TM->Cols; ++c)
        {
            if (APCBaseUnitCharacter* U = TM->GetFieldUnit(c, r))
            {
                if (U->GetTeamIndex() == HostSeatIndex) ++HostCnt;
                else if (U->GetTeamIndex() == CreepTeam) ++CreepCnt;
            }
        }
    }

    Pair.HostAlive  = HostCnt;
    Pair.GuestAlive = CreepCnt;
    Pair.bRunning   = true;

    UE_LOG(LogTemp, Log, TEXT("[PvE Start] Seat=%d Pair=%d FieldSaved=%d Creeps=%d"),
        HostSeatIndex, PairIndex, Pair.HostFieldSnapshot.Field.Num(), Pair.PvECreeps.Num());

    return PairIndex;
}

void APCCombatManager::FinishPvEBattleForSeat(int32 HostSeatIndex)
{
	if (!IsAuthority()) return;

	// Seat로 PairIndex 찾기 (bIsPvE && HostSeat 일치 && bRunning 여부 체크 추천)
	int32 PairIndex = INDEX_NONE;
	for (int32 i = 0; i < Pairs.Num(); ++i)
	{
		if (!Pairs[i].bIsPvE) continue;
		if (!Pairs[i].Host.IsValid()) continue;
		if (Pairs[i].Host->BoardSeatIndex == HostSeatIndex)
		{
			PairIndex = i; break;
		}
	}
	if (PairIndex == INDEX_NONE) return;

	FCombatManager_Pair& Pair = Pairs[PairIndex];
	APCCombatGameState* GS = GetWorld() ? GetWorld()->GetGameState<APCCombatGameState>() : nullptr;
	APCCombatBoard* Host = GS ? GS->GetBoardBySeat(HostSeatIndex) : nullptr;

	if (!GS || !Host || !Host->TileManager)
	{
		UE_LOG(LogTemp, Error, TEXT("[PvE Finish] Seat=%d Board/TM null"), HostSeatIndex);
		return;
	}

	UPCTileManager* TM = Host->TileManager;

	// 1) 크립 제거
	int32 Removed = 0;
	for (auto& WU : Pair.PvECreeps)
	{
		if (APCBaseUnitCharacter* Creep = WU.Get())
		{
			if (RemoveUnitFromAny(TM, Creep)) ++Removed;
		}
	}

	// 2) Host 필드 원복 (Seat를 키로 현재 보드에 복구)
	UE_LOG(LogTemp, Log, TEXT("[PvE Finish] Seat=%d Pair=%d RemoveCreeps=%d RestoreSlots=%d"),
		HostSeatIndex, PairIndex, Removed, Pair.HostFieldSnapshot.Field.Num());

	RestoreSnapShotPvE(Pair.HostFieldSnapshot);

	// 3) 언바인드 및 런타임 정리
	UnbindAllForPair(PairIndex);
	Pair.HostFieldSnapshot.Reset();
	Pair.bRunning = false;
}

void APCCombatManager::FinishAllPve()
{
	if (!IsAuthority()) return;

	// Pairs를 직접 순회하면서 PvE 페어만 종료
	TArray<int32> Seats;
	for (const auto& Pair : Pairs)
	{
		if (Pair.bIsPvE && Pair.Host.IsValid())
		{
			Seats.AddUnique(Pair.Host->BoardSeatIndex);
		}
	}
	for (int32 Seat : Seats)
	{
		FinishPvEBattleForSeat(Seat);
	}
}

bool APCCombatManager::GetCurrentStageRoundOne(int32& OutStageOne, int32& OutRoundOne) const
{
	if (APCCombatGameState* PCGS = GetWorld() ? GetWorld()->GetGameState<APCCombatGameState>() : nullptr)
	{
		const auto& RT = PCGS->GetStageRunTime();
		OutStageOne = RT.StageIdx;
		OutRoundOne = RT.RoundIdx;
		return (OutStageOne > 0 && OutRoundOne >0);
	}
	return false;
}

FGameplayTag APCCombatManager::GetCreepTagForStageRound(int32 StageOne, int32 RoundOne) const
{
	return UnitGameplayTags::Unit_Type_Creep_MinionLv1;
}

int32 APCCombatManager::GetCreepLevelForStageRound(int32 StageOne, int32 RoundOne) const
{
	return 1;
}

bool APCCombatManager::PlaceOrNearest(UPCTileManager* TM, int32 Y, int32 X, APCBaseUnitCharacter* Creep) const
{
	if (!TM || !Creep)
		return false;

	// 정확한 자리 가능하면 바로 배치
	if (TM->IsInRange(Y, X) && TM->IsTileFree(Y, X))
	{
		return TM->PlaceUnitOnField(Y, X, Creep, ETileFacing::Enemy);
	}

	// 못 놓으면 반경 확장하며 “십자-다이아” 형태로 인접 탐색
	const int32 RadiusMax = 3; // 필요 시 조정
	for (int32 R = 1; R <= RadiusMax; ++R)
	{
		for (int32 dy = -R; dy <= R; ++dy)
		{
			const int32 dxs[2] = { R - FMath::Abs(dy), -(R - FMath::Abs(dy)) };
			for (int32 k = 0; k < 2; ++k)
			{
				const int32 ny = Y + dy;
				const int32 nx = X + dxs[k];
				if (TM->IsInRange(ny, nx) && TM->IsTileFree(ny, nx))
					return TM->PlaceUnitOnField(ny, nx, Creep, ETileFacing::Enemy);
			}
		}
	}

	return false;
}

APCBaseUnitCharacter* APCCombatManager::SpawnCreepAt(APCCombatBoard* Board, int32 StageOne, int32 RoundOne,
                                                     const FIntPoint& YX) const
{
	if (!Board || !IsValid(Board->TileManager))
		return nullptr;

	UPCTileManager* TM = Board->TileManager;

	// GameMode 규칙과 동일:
	const int32      CreepTeam = GetCreepTeamIndexForBoard(Board);
	const FGameplayTag CreepTag = GetCreepTagForStageRound(StageOne, RoundOne);
	const int32      CreepLevel = GetCreepLevelForStageRound(StageOne, RoundOne);

	UPCUnitSpawnSubsystem* SpawnSubsystem = GetWorld()->GetSubsystem<UPCUnitSpawnSubsystem>();
	if (!SpawnSubsystem)
		return nullptr;

	// GameMode 예시: SpawnUnitByTag(CreepTag, CreepTeam, Level, Instigator)
	// Instigator는 CombatManager(this)로 넘겨도 되고, nullptr로 두어도 됨 (서브시스템 시그니처에 맞춰 조정)
	APCBaseUnitCharacter* Unit = SpawnSubsystem->SpawnUnitByTag(CreepTag, CreepTeam, CreepLevel);
	if (!Unit)
		return nullptr;

	// 팀 보장
	if (Unit->GetTeamIndex() != CreepTeam)
		Unit->SetTeamIndex(CreepTeam);

	Unit->SetActorHiddenInGame(false);
	Unit->SetActorEnableCollision(true);

	// (Y,X) 자리 또는 주변 빈칸에 배치 (적 방향)
	if (!PlaceOrNearest(TM, YX.X, YX.Y, Unit))
	{
		// 자리가 끝내 없으면 정리
		Unit->Destroy();
		return nullptr;
	}

	return Unit;
}

void APCCombatManager::CountAliveOnHostBoardForPair(int32 PairIndex)
{
	auto& Pair = Pairs[PairIndex];
	APCCombatBoard* Host = Pair.Host.Get();
	APCCombatBoard* Guest = Pair.Guest.Get();

	if (!Host || !Host->TileManager)
	{
		Pair.HostAlive = 0;
		Pair.GuestAlive = 0;
		return;
	}

	const int32 HostSeat = Host->BoardSeatIndex;
	const int32 GuestSeat = Guest ? Guest->BoardSeatIndex : INDEX_NONE;

	int32 HostCnt = 0;
	int32 GuestCnt = 0;

	UPCTileManager* TM = Host->TileManager;
	for (int32 y = 0; y < TM->Cols; ++y)
	{
		for (int32 x = 0; x < TM->Rows; ++x)
		{
			if (APCBaseUnitCharacter* Unit = TM->GetFieldUnit(y,x))
			{
				const int32 Team = Unit->GetTeamIndex();
				if (Team == HostSeat)
				{
					++HostCnt;
				}
				else if (Team == GuestSeat)
				{
					++GuestCnt;
				}
			}
		}
	}

	Pair.HostAlive = HostCnt;
	Pair.GuestAlive = GuestCnt;
}

void APCCombatManager::BindUnitOnBoardForPair(int32 PairIndex)
{
	auto& Pair = Pairs[PairIndex];
	APCCombatBoard* Host = Pair.Host.Get();
	APCCombatBoard* Guest = Pair.Guest.Get();
	if (!Host || !Host->TileManager)
		return;

	const int32 HostSeat = Host->BoardSeatIndex;
	const int32 GuestSeat = Guest ? Guest->BoardSeatIndex : INDEX_NONE;

	UPCTileManager* TM = Host->TileManager;

	auto TryBind = [&](APCBaseUnitCharacter* Unit)
	{
		if (!Unit)
			return;

		if (UnitToPairIndex.Contains(Unit))
			return;

		Unit->OnUnitDied.AddDynamic(this, &APCCombatManager::OnAnyUnitDied);
		UnitToPairIndex.Add(Unit,PairIndex);
	};

	for (int32 y = 0; y < TM->Cols; ++y)
	{
		for (int32 x = 0; x < TM->Rows; ++x)
		{
			TryBind(TM->GetFieldUnit(y,x));
		}
	}
}

void APCCombatManager::UnbindAllForPair(int32 PairIndex)
{
	auto& Pair = Pairs[PairIndex];

	auto TryUnbind = [&](APCBaseUnitCharacter* Unit)
	{
		if (!Unit) return;
		if (UnitToPairIndex.Remove(Unit) > 0)
		{
			Unit->OnUnitDied.RemoveDynamic(this, &APCCombatManager::OnAnyUnitDied);
		}
	};

	if (APCCombatBoard* Host = Pair.Host.Get())
	{
		if (UPCTileManager* TM = Host->TileManager)
		{
			for (int32 y = 0; y < TM->Cols; ++y)
			{
				for (int32 x = 0; x < TM->Rows; ++x)
				{
					TryUnbind(TM->GetFieldUnit(y, x));
				}
			}
		}
	}

	for (const auto& WU : Pair.MovedUnits)
	{
		if (APCBaseUnitCharacter* Unit = WU.Get())
		{
			TryUnbind(Unit);
		}
	}

	// PvE 크립도 언바인드
	for (const auto& WU : Pair.PvECreeps)
	{
		if (APCBaseUnitCharacter* Unit = WU.Get())
		{
			TryUnbind(Unit);
		}
	}

	Pair.DeadUnits.Reset();
}


void APCCombatManager::OnAnyUnitDied(APCBaseUnitCharacter* Unit)
{
	if (!IsAuthority() || !Unit) return;

	int32* PairIndexPtr = UnitToPairIndex.Find(Unit);
	if (!PairIndexPtr) return;

	const int32 PairIndex = *PairIndexPtr;
	if (!Pairs.IsValidIndex(PairIndex)) return;

	auto& Pair = Pairs[PairIndex];
	if (!Pair.bRunning) return;

	if (Pair.DeadUnits.Contains(Unit)) return;
	Pair.DeadUnits.Add(Unit);

	APCCombatBoard* Host  = Pair.Host.Get();
	APCCombatBoard* Guest = Pair.Guest.Get();
	if (!Host) return;

	const int32 HostSeat  = Host->BoardSeatIndex;
	const int32 GuestSeat = Guest ? Guest->BoardSeatIndex : INDEX_NONE;

	if (Pair.bIsPvE)
	{
		const int32 CreepTeam = GetCreepTeamIndexForBoard(Host);
		const int32 Team = Unit->GetTeamIndex();

		if (Team == HostSeat)
		{
			Pair.HostAlive = FMath::Max(0, Pair.HostAlive - 1);
		}
		else if (Team == CreepTeam || Pair.PvECreeps.Contains(Unit))
		{
			Pair.GuestAlive = FMath::Max(0, Pair.GuestAlive - 1);
		}
	}
	else
	{
		const int32 Team = Unit->GetTeamIndex();
		if (Team == HostSeat)
		{
			Pair.HostAlive = FMath::Max(0, Pair.HostAlive - 1);
		}
		else if (Team == GuestSeat)
		{
			Pair.GuestAlive = FMath::Max(0, Pair.GuestAlive - 1);
		}
	}

	CheckPairVictory(PairIndex);
}

void APCCombatManager::CheckPairVictory(int32 PairIndex)
{
	auto& Pair = Pairs[PairIndex];
	if (!Pair.bRunning)
		return;

	if (Pair.HostAlive <= 0 && Pair.GuestAlive <= 0)
	{
		ResolvePairResult(PairIndex,false);
		return;
	}

	if (Pair.HostAlive <= 0)
	{
		ResolvePairResult(PairIndex, false);
		return;
	}

	if (Pair.GuestAlive <= 0)
	{
		ResolvePairResult(PairIndex, true);
	}
	
}

void APCCombatManager::ResolvePairResult(int32 PairIndex, bool bHostWon)
{
	if (!HasAuthority() || !Pairs.IsValidIndex(PairIndex)) return;

	auto& Pair = Pairs[PairIndex];

	// PvE는 데미지 이벤트 생략(필요 시 규칙 추가)
	if (Pair.bIsPvE)
	{
		UnbindAllForPair(PairIndex);
		Pair.bRunning = false;

		const int32 HostSeat = Pair.Host.IsValid() ? Pair.Host->BoardSeatIndex : INDEX_NONE;
		OnCombatPairResult.Broadcast(bHostWon ? HostSeat : INDEX_NONE,
									 bHostWon ? INDEX_NONE : HostSeat,
									 Pair.HostAlive, Pair.GuestAlive);
		return;
	}

	// PvP
	const int32 HostSeat  = Pair.Host.IsValid()  ? Pair.Host->BoardSeatIndex  : INDEX_NONE;
	const int32 GuestSeat = Pair.Guest.IsValid() ? Pair.Guest->BoardSeatIndex : INDEX_NONE;
	if (HostSeat == INDEX_NONE || GuestSeat == INDEX_NONE) return;

	APawn* AttackerPawn = FindPawnBySeat(bHostWon ? HostSeat : GuestSeat);
	APawn* DefenderPawn = FindPawnBySeat(bHostWon ? GuestSeat : HostSeat);
	if (!AttackerPawn || !DefenderPawn) return;

	const int32 StageIndex      = GetCurrentStageIndex();
	const int32 StageBaseDamage = GetStageBaseDamageFromDT(StageIndex);
	const int32 Alive           = bHostWon ? Pair.HostAlive : Pair.GuestAlive;
	const int32 Damage          = FMath::Max(1, StageBaseDamage + Alive);

	FGameplayEventData DamageData;
	DamageData.EventTag       = DamageEventTag;
	DamageData.EventMagnitude = Damage;
	DamageData.Instigator     = AttackerPawn;
	DamageData.Target         = DefenderPawn;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(AttackerPawn, DamageEventTag, DamageData);

	UnbindAllForPair(PairIndex);
	Pair.bRunning = false;

	const int32 WinnerSeat = bHostWon ? HostSeat : GuestSeat;
	const int32 LoserSeat  = bHostWon ? GuestSeat : HostSeat;
	OnCombatPairResult.Broadcast(WinnerSeat, LoserSeat, Pair.HostAlive, Pair.GuestAlive);
}

void APCCombatManager::OnUnitSpawnedDuringBattle(APCBaseUnitCharacter* Unit, int32 SeatIndex)
{
	if (!IsAuthority() || !Unit)
		return;

	const int32 PairIdx = FindRunningPairIndexBySeat(SeatIndex);
	if (PairIdx == INDEX_NONE)
		return;

	auto& Pair = Pairs[PairIdx];

	// 전장은 HOST 보드
	APCCombatBoard* BattleBoard = Pair.Host.Get();
	if (!BattleBoard || !BattleBoard->TileManager)
		return;

	const bool bEnemySide = (Pair.Guest.IsValid() && SeatIndex == Pair.Guest->BoardSeatIndex);
	UPCTileManager* TM = BattleBoard->TileManager;

	// 위치 : 전장 보드의 벤치 첫 빈칸
	const int32 BenchIdx = FindFirstFreeBenchIndex(TM,bEnemySide);
	if (BenchIdx != INDEX_NONE)
	{
		TM->PlaceUnitOnBench(BenchIdx, Unit, bEnemySide ? ETileFacing::Enemy : ETileFacing::Friendly);
	}

	Pair.NewUnitDuringBattle.FindOrAdd(SeatIndex).Add(Unit);
}

int32 APCCombatManager::FindRunningPairIndexBySeat(int32 SeatIndex) const
{
	for (int32 i = 0; i < Pairs.Num(); ++i)
	{
		const auto& Pair = Pairs[i];
		const int32 HostSeat = Pair.Host.IsValid() ? Pair.Host->BoardSeatIndex : INDEX_NONE;
		const int32 GuestSeat = Pair.Guest.IsValid() ? Pair.Guest->BoardSeatIndex : INDEX_NONE;
		if (!Pair.bRunning)
			continue;
		if (SeatIndex == HostSeat || SeatIndex == GuestSeat)
			return i;
	}
	return INDEX_NONE;
}

int32 APCCombatManager::FindFirstFreeBenchIndex(UPCTileManager* TM, bool bEnemySide) const
{
	if (!TM) return INDEX_NONE;
	const int32 N = TM->BenchSlotsPerSide;
	for (int32 local = 0; local < N; ++local)
	{
		const int32 idx = TM->MakeGlobalBenchIndex(bEnemySide, local); // ★ 중요
		if (idx != INDEX_NONE && TM->GetBenchUnit(idx) == nullptr)
			return idx;
	}
	return INDEX_NONE;
}

static FName MakeStageRowName(int32 Stage)
{
	return FName(*FString::Printf(TEXT("Stage_%d"), Stage));
}

int32 APCCombatManager::GetCurrentStageIndex() const
{
	if (const APCCombatGameState* GameState = GetWorld() ? GetWorld()->GetGameState<APCCombatGameState>() : nullptr)
	{
		return GameState->GetStageRunTime().StageIdx;
	}

	return INDEX_NONE;
}

int32 APCCombatManager::GetStageBaseDamageFromDT(int32 StageIdx) const
{
	if (!StageDamageTable)
	{
		return GetStageBaseDamageDefault(StageIdx);
	}

	static const FString Ctx(TEXT("StageBaseDamageLookup"));
	if (const FStageDamage* Row = StageDamageTable->FindRow<FStageDamage>(MakeStageRowName(StageIdx), Ctx, false))
	{
		return Row->BaseDamage;
	}

	if (bClampToLastRow)
	{
		TArray<FName> Names = StageDamageTable->GetRowNames();
		if (Names.Num() > 0)
		{
			Names.Sort(FNameLexicalLess());
			if (const FStageDamage* Last = StageDamageTable->FindRow<FStageDamage>(Names.Last(), Ctx, false))
			{
				return Last->BaseDamage;
			}
		}
	}

	return 0;
}

int32 APCCombatManager::GetStageBaseDamageDefault(int32 StageIdx) const
{
	switch (StageIdx)
	{
	case 1:  return 0;
	case 2:  return 2;
	case 3:  return 5;
	case 4:  return 8;
	case 5:  return 10;
	case 6:  return 12;
	case 7:  return 17;
	default: return 150;
	}
}
