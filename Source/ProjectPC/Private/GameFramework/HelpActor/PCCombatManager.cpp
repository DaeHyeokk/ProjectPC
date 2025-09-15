


#include "GameFramework/HelpActor/PCCombatManager.h"

#include "EngineUtils.h"
#include "Character/Unit/PCHeroUnitCharacter.h"
#include "Controller/Player/PCCombatPlayerController.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/GameState/PCCombatGameState.h"
#include "GameFramework/HelpActor/PCCombatBoard.h"
#include "GameFramework/HelpActor/Component/PCTileManager.h"
#include "GameFramework/PlayerState/PCPlayerState.h"


APCCombatManager::APCCombatManager()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = false;
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

	if (Players.IsEmpty())
	{
		Pairs.Reset();
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
		Pairs.Add(Pair);
	}
}

void APCCombatManager::StartAllBattle()
{
	if (!IsAuthority())
		return;
	for (FCombatManager_Pair& InPair : Pairs)
	{
		APCCombatBoard* Host = InPair.Host.Get();
		APCCombatBoard* Guest = InPair.Guest.Get();
		if (!Host || !Host->TileManager)
		{
			continue;
		}

		if (!Guest || !Guest->TileManager)
		{
			continue;
		}

		TakeSnapshot(Guest, InPair.GuestSnapShot);

		UPCTileManager* HostTileManager = Host->TileManager;
		UPCTileManager* GuestTileManager = Guest->TileManager;

		InPair.MovedUnits.Reset();
		for (const auto& FiledSlot : InPair.GuestSnapShot.Field)
		{
			if (FiledSlot.Unit.IsValid())
			{
				InPair.MovedUnits.Add(FiledSlot.Unit);
			}
		}
		for (const auto& BenchSlot : InPair.GuestSnapShot.Bench)
		{
			if (BenchSlot.Unit.IsValid())
			{
				InPair.MovedUnits.Add(BenchSlot.Unit);
			}
		}

		GuestTileManager->MoveUnitsMirroredTo(HostTileManager, bMirrorRows, bMirrorCols, bIncludeBench);
		
		for (auto& UnitWeak : InPair.MovedUnits)
		{
			if (auto* U = UnitWeak.Get())
			{
				UE_LOG(LogTemp, Log, TEXT("Unit=%s OnBoard=%s"),
					*U->GetName(),
					U->GetOnCombatBoard() ? *U->GetOnCombatBoard()->GetName() : TEXT("NULL"));
			}
		}
		
		if (APCCombatGameState* PCGameState = GetWorld()->GetGameState<APCCombatGameState>())
		{
			PCGameState->SetGameStateTag(GameStateTags::Game_State_Combat_Preparation);
		}
		
	}
}

void APCCombatManager::FinishAllBattle()
{
	if (!IsAuthority())
		return;
	for (const FCombatManager_Pair InPair : Pairs)
	{
		APCCombatBoard* Host = InPair.Host.Get();
		APCCombatBoard* Guest = InPair.Guest.Get();
		UPCTileManager* HostTileManager = Host->TileManager;
		UPCTileManager* GuestTileManager = Guest->TileManager;

		if (!Guest || ! GuestTileManager) continue;

		if (HostTileManager)
		{
			for (const auto& WeakUnit : InPair.MovedUnits)
			{
				if (APCBaseUnitCharacter* Unit = WeakUnit.Get())
				{
					RemoveUnitFromAny(HostTileManager, Unit);
				}
			}
		}
		RestoreSnapshot(InPair.GuestSnapShot);
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

	for (const auto& FieldSlot : Snap.Field)
	{
		if (APCBaseUnitCharacter* InUnit = FieldSlot.Unit.Get())
		{
			TileManager->PlaceUnitOnField(FieldSlot.Col, FieldSlot.Row, InUnit);
		}
	}

	for (const auto& BenchSlot : Snap.Bench)
	{
		if (APCBaseUnitCharacter* InUnit = BenchSlot.Unit.Get())
		{
			TileManager->PlaceUnitOnBench(BenchSlot.Index, InUnit);
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

	FCombatManager_Pair& Pair = Pairs[PairIndex];
	APCCombatBoard* Host = Pair.Host.Get();
	APCCombatBoard* Guest = Pair.Guest.Get();

	if (!Host)
		return;

	const int32 HostSeat = Host->BoardSeatIndex;
	const int32 GuestSeat = Guest->BoardSeatIndex;

	const FTransform T_Player = Host->GetPlayerSeatTransform();
	const FTransform T_Enemy = Host->GetEnemySeatTransform();

	if (APawn* PHost = FindPawnBySeat(HostSeat))
	{
		TeleportPlayerToTransform(PHost, T_Player);
	}
	if (Guest)
	{
		if (APawn* PGuest = FindPawnBySeat(GuestSeat))
		{
			TeleportPlayerToTransform(PGuest, T_Enemy);
			PGuest->SetActorRotation(GuestRotation);
		}
	}

	if (APCPlayerState* HostPlayerState = FindPlayerStateBySeat(HostSeat))
	{
		FocusCameraToBoard(HostPlayerState->SeatIndex, HostSeat, true, Blend);
	}

	if (Guest)
	{
		if (APCPlayerState* GuestPlayerState = FindPlayerStateBySeat(GuestSeat))
		{
			FocusCameraToBoard(GuestPlayerState->SeatIndex, HostSeat, true, Blend);
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
		}
	}

	if (APCPlayerState* HostPlayerState = FindPlayerStateBySeat(HostSeat))
	{
		FocusCameraToBoard(HostPlayerState->SeatIndex, HostSeat, false, Blend);
	}

	if (Guest)
	{
		if (APCPlayerState* GuestPlayerState = FindPlayerStateBySeat(GuestSeat))
		{
			FocusCameraToBoard(GuestPlayerState->SeatIndex, GuestSeat, false, Blend);
		}
	}
}