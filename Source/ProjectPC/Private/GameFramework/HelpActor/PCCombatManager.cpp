


#include "GameFramework/HelpActor/PCCombatManager.h"

#include "EngineUtils.h"
#include "Character/UnitCharacter/PCHeroUnitCharacter.h"
#include "Controller/Player/PCCombatPlayerController.h"
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
		UE_LOG(LogTemp, Warning, TEXT("[CM] BuildRandomPairs on non-authority"));
		return;
	}

	TArray<APCCombatBoard*> Boards;
	for (TActorIterator<APCCombatBoard> It(GetWorld()); It; ++It)
	{
		if (It->TileManager)
		{
			Boards.Add(*It);
		}
	}
	if (Boards.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[CM] No boards"))
		return;
	}

	Boards.Sort([](const APCCombatBoard& A, const APCCombatBoard& B)
	{
		return A.BoardSeatIndex < B.BoardSeatIndex;
	});
	FRandomStream RNG(RandomSeed);
	for (int32 i = 0; i < Boards.Num(); ++i)
	{
		int32 SwapIdx = RNG.RandRange(i, Boards.Num() - 1);
		Boards.Swap(i, SwapIdx);
	}

	Pairs.Reset();
	for (int32 i = 0; i < Boards.Num(); i += 2)
	{
		FCombatManager_Pair InPairs;
		InPairs.Host = Boards[i];
		if (i + 1 < Boards.Num())
		{
			InPairs.Guest = Boards[i + 1];
		}
		else
		{
			InPairs.Guest = nullptr;
		}
		InPairs.GuestSnapShot.Reset();
		InPairs.MovedUnits.Reset();
		Pairs.Add(InPairs);
	}
	// 로그
	UE_LOG(LogTemp, Log, TEXT("[CM] Built %d pairs (boards=%d)"), Pairs.Num(), Boards.Num());
	for (int32 k=0;k<Pairs.Num();++k)
	{
		auto H = Pairs[k].Host.Get();
		auto G = Pairs[k].Guest.Get();
		UE_LOG(LogTemp, Log, TEXT("  Pair %d: Host S=%d  Guest %s"), k,
			H ? H->BoardSeatIndex : -1,
			G ? *FString::Printf(TEXT("S=%d"), G->BoardSeatIndex) : TEXT("BYE"));
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
		if (!Host || !Host->TileManager) continue;

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
				if (APCHeroUnitCharacter* Unit = WeakUnit.Get())
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
			if (APCHeroUnitCharacter* InUnit = TileManager->GetFieldUnit(c,r))
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
		if (APCHeroUnitCharacter* InUnit = TileManager->GetBenchUnit(i))
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
		if (APCHeroUnitCharacter* InUnit = FieldSlot.Unit.Get())
		{
			TileManager->PlaceUnitOnField(FieldSlot.Col, FieldSlot.Row, InUnit);
		}
	}

	for (const auto& BenchSlot : Snap.Bench)
	{
		if (APCHeroUnitCharacter* InUnit = BenchSlot.Unit.Get())
		{
			TileManager->PlaceUnitOnBench(BenchSlot.Index, InUnit);
		}
	}
}

bool APCCombatManager::RemoveUnitFromAny(UPCTileManager* TileManager, APCHeroUnitCharacter* Unit)
{
	if (!TileManager || !Unit) return false;

	for (int32 r = 0; r < TileManager->Rows; ++r)
	{
		for (int32 c = 0; c < TileManager->Cols; ++c)
		{
			if (TileManager->GetFieldUnit(c, r) == Unit)
			{
				TileManager->RemoveFromField(c,r);
				return true;
			}
		}
	}

	for (int32 i = 0; i < TileManager->Bench.Num(); ++i)
	{
		if (TileManager->GetBenchUnit(i) == Unit)
		{
			TileManager->RemoveFromBench(i);
			return true;
		}
	}
	return false;
}

void APCCombatManager::Server_TravelFocusCamera(APCPlayerState* PCPlayerStateA, int32 BoardSeatIndexA,
	APCPlayerState* PCPlayerStateB, int32 BoardSeatIndexB, float Blend)
{
	if (auto* PCPlayerControllerA = PCPlayerStateA ? Cast<APCCombatPlayerController>(PCPlayerStateA->GetOwner()) : nullptr )
	{
		PCPlayerControllerA->ClientFocusBoardBySeatIndex(BoardSeatIndexA, true, Blend);
	}
	if (auto* PCPlayerControllerB = PCPlayerStateB ? Cast<APCCombatPlayerController>(PCPlayerStateB->GetOwner()) : nullptr)
	{
		PCPlayerControllerB->ClientFocusBoardBySeatIndex(BoardSeatIndexB, true, Blend);
	}
}

void APCCombatManager::Server_ReturnFocusCamera(APCPlayerState* PCPlayerStateA, int32 BoardSeatIndexA,
	APCPlayerState* PCPlayerStateB, int32 BoardSeatIndexB, float Blend)
{
	if (auto* PCPlayerControllerA = PCPlayerStateA ? Cast<APCCombatPlayerController>(PCPlayerStateA->GetOwner()) : nullptr )
	{
		PCPlayerControllerA->ClientFocusBoardBySeatIndex(BoardSeatIndexA, false, Blend);
	}
	if (auto* PCPlayerControllerB = PCPlayerStateB ? Cast<APCCombatPlayerController>(PCPlayerStateB->GetOwner()) : nullptr)
	{
		PCPlayerControllerB->ClientFocusBoardBySeatIndex(BoardSeatIndexB, false, Blend);
	}
}