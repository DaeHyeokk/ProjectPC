


#include "GameFramework/HelpActor/PCCombatManager.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "EngineUtils.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "Character/Unit/PCHeroUnitCharacter.h"
#include "Controller/Player/PCCombatPlayerController.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/GameState/PCCombatGameState.h"
#include "GameFramework/HelpActor/PCCombatBoard.h"
#include "GameFramework/HelpActor/PCPlayerBoard.h"
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
		case 2: Out = { FIntPoint(5,2), FIntPoint(5,4) }; return true;
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

	for (int32 PairIndex = 0; PairIndex < Pairs.Num(); ++PairIndex)
	{
		auto& Pair  = Pairs[PairIndex];
		APCCombatBoard* Host  = Pair.Host.Get();
		APCCombatBoard* Guest = Pair.Guest.Get();
		if (!Host || !Guest) continue;

		UPCTileManager* HostTM = Host->TileManager;
		if (!HostTM) continue;

		const int32 HostSeat  = Host->BoardSeatIndex;
		const int32 GuestSeat = Guest->BoardSeatIndex;

		// === PlayerBoard 찾기
		APCPlayerBoard* HostPB  = FindPlayerBoardBySeat(HostSeat);
		APCPlayerBoard* GuestPB = FindPlayerBoardBySeat(GuestSeat);
		if (!HostPB || !GuestPB) continue;

		// === PlayerBoard 화면/공간 이동: Host 전장 위로 부착 (둘 다)
		GuestPB->AttachToCombatBoard(Host, true);

		// === PlayerBoard 필드 스냅샷 (복귀용)
		TakeFieldSnapShot(HostPB,  Pair.HostSnapShot);
		TakeFieldSnapShot(GuestPB, Pair.GuestSnapShot);

		// === 전장(TM) 초기화
		HostTM->ClearAll();

		// === 배치: Host 필드(그대로, Friendly) + Guest 필드(미러 적용, Enemy)
		PlacePlayerBoardToTM(HostPB,  HostTM, false,false, ETileFacing::Friendly);
		PlacePlayerBoardToTM(GuestPB, HostTM, bMirrorRows, bMirrorCols, ETileFacing::Enemy);

		HostTM->DebugLogField(true,true,FString("CombatManager"));

		// 생존 수 카운트 + 바인딩
		CountAliveOnHostBoardForPair(PairIndex);
		BindUnitOnBoardForPair(PairIndex);

		Pair.bRunning = true;
		Pair.bIsPvE   = false;
	}
}

void APCCombatManager::FinishAllBattle()
{
	if (!IsAuthority())
		return;
	

	for (int32 PairIndex = 0; PairIndex < Pairs.Num(); ++PairIndex)
	{
		auto& Pair = Pairs[PairIndex];
		APCCombatBoard* Host  = Pair.Host.Get();
		APCCombatBoard* Guest = Pair.Guest.Get();
		if (!Host) continue;

		// 언바인드
		UnbindAllForPair(PairIndex);

		UPCTileManager* HostTM = Host->TileManager;
		if (HostTM)
		{
			// 전장 필드 비우기
			HostTM->ClearAll();
		}
		
		// === PlayerBoard 분리(원래 자리로 복귀 + 벤치 스냅)
		if (APCPlayerBoard* HostPB = FindPlayerBoardBySeat(Host->BoardSeatIndex))
			HostPB->DetachFromCombatBoard();
		if (Guest && Guest->BoardSeatIndex >= 0)
		{
			if (APCPlayerBoard* GuestPB = FindPlayerBoardBySeat(Guest->BoardSeatIndex))
				GuestPB->DetachFromCombatBoard();
		}

		// === PlayerBoard 필드 복구
		RestoreFieldSnapShot(Pair.HostSnapShot);
		RestoreFieldSnapShot(Pair.GuestSnapShot);
		Pair.HostSnapShot.Reset();
		Pair.GuestSnapShot.Reset();
		
		Pair.bRunning = false;
	}
}

void APCCombatManager::PlacePlayerBoardToTM(APCPlayerBoard* PlayerBoard, UPCTileManager* TM, bool MirrorRows,
	bool MirrorCols, ETileFacing Facing)
{
	if (!IsValid(PlayerBoard) || !IsValid(TM)) return;
	
	// 1) PB 필드 스냅샷 (Y=Col, X=Row)
	struct FCaptured { int32 Y; int32 X; TWeakObjectPtr<APCBaseUnitCharacter> Unit; };
	TArray<FCaptured> Capture;
	Capture.Reserve(PlayerBoard->Rows * PlayerBoard->Cols);

	for (int32 y = 0; y < PlayerBoard->Cols; ++y)
	{
		for (int32 x = 0; x < PlayerBoard->Rows; ++x)
		{
			const int32 i = PlayerBoard->IndexOf(y, x); // PB: IndexOf(Y,X) = Y*Rows + X
			if (!PlayerBoard->PlayerField.IsValidIndex(i)) continue;

			if (APCBaseUnitCharacter* U = PlayerBoard->PlayerField[i].Unit)
			{
				Capture.Add({ y, x, U });
			}
		}
	}

	// 2) TM 기준으로 좌표 미러 계산 후 배치
	for (const auto& E : Capture)
	{
		// TM의 크기를 기준으로 미러링
		const int32 dstX = MirrorRows ? (TM->Rows - 1 - E.X) : E.X; // Row
		const int32 dstY = MirrorCols ? (TM->Cols - 1 - E.Y) : E.Y; // Col

		// 범위 체크 (PB/TM 크기 다를 수 있음)
		if (dstY < 0 || dstY >= TM->Cols || dstX < 0 || dstX >= TM->Rows)
			continue;

		if (APCBaseUnitCharacter* Unit = E.Unit.Get())
		{
			TM->EnsureExclusive(Unit); // 다른 TM/슬롯에 있던 경우 분리
			const bool ok = TM->PlaceUnitOnField(dstY, dstX, Unit, Facing);

			// 팀 보장
			if (ok && Unit->GetTeamIndex() != PlayerBoard->PlayerIndex)
			{
				Unit->SetTeamIndex(PlayerBoard->PlayerIndex);
			}

			UE_LOG(LogTemp, Warning,
				TEXT("[PlacePB->TM] Place success dst=(Y=%d,X=%d) Unit=%s, UnitTeamIndex = %d"),
				dstY, dstX, *GetNameSafe(E.Unit.Get()), E.Unit.Get()->GetTeamIndex());
				
			if (!ok)
			{
				UE_LOG(LogTemp, Warning,
					TEXT("[PlacePB->TM] Place failed dst=(Y=%d,X=%d) Unit=%s"),
					dstY, dstX, *GetNameSafe(E.Unit.Get()));
			}
		}		
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

	APCPlayerBoard* HostPB = FindPlayerBoardBySeat(HostSeatIndex);
	if (!HostPB) return INDEX_NONE;

	const int32 PairIndex = Pairs.AddDefaulted();
	FCombatManager_Pair& Pair = Pairs[PairIndex];
	Pair.ResetRuntime();
	Pair.Host     = HostBoard;
	Pair.Guest    = nullptr;
	Pair.bIsPvE   = true;
	Pair.bRunning = true;

	// 1) 보드 스냅샷 저장
	HostPB->MakeSnapshot(Pair.PvESnapShot);

	// 2) 전장 초기화
	UPCTileManager* TM = HostBoard->TileManager;
	TM->ClearAll();

	// 3) 보드 → TM 배치(미러 없음, Friendly)
	for (int32 y=0; y<HostPB->Cols; ++y)
	{
		for (int32 x=0; x<HostPB->Rows; ++x)
		{
			const int32 i = HostPB->IndexOf(y,x);
			if (!HostPB->PlayerField.IsValidIndex(i)) continue;

			if (APCBaseUnitCharacter* U = HostPB->PlayerField[i].Unit)
			{
				TM->EnsureExclusive(U);
				TM->PlaceUnitOnField(y, x, U, ETileFacing::Friendly);
			}
		}
	}

	// 4) 크립 스폰/배치
	int32 StageOne=0, RoundOne=0;
	if (GetCurrentStageRoundOne(StageOne, RoundOne))
	{
		TArray<FIntPoint> Points; BuildCreepPoints(StageOne, RoundOne, Points);
		for (const FIntPoint& YX : Points)
			if (APCBaseUnitCharacter* Creep = SpawnCreepAt(HostBoard, StageOne, RoundOne, YX))
			{ Pair.PvECreeps.Add(Creep); UnitToPairIndex.Add(Creep, PairIndex); Creep->OnUnitDied.AddDynamic(this, &ThisClass::OnAnyUnitDied); }
	}

	CountAliveOnHostBoardForPair(PairIndex);
	BindUnitOnBoardForPair(PairIndex);
	return PairIndex;
}

void APCCombatManager::FinishPvEBattleForSeat(int32 HostSeatIndex)
{
	if (!IsAuthority()) return;

	int32 PairIndex = INDEX_NONE;
	for (int32 i=0;i<Pairs.Num();++i)
		if (Pairs[i].bIsPvE && Pairs[i].Host.IsValid() && Pairs[i].Host->BoardSeatIndex == HostSeatIndex)
		{ PairIndex = i; break; }
	if (PairIndex == INDEX_NONE) return;

	FCombatManager_Pair& Pair = Pairs[PairIndex];
	APCCombatBoard* Host = Pair.Host.Get();
	if (!Host || !Host->TileManager) return;

	UPCTileManager* TM = Host->TileManager;

	// 1) 크립 제거(필드에서만 빼면 충분)
	for (auto& WU : Pair.PvECreeps)
		if (APCBaseUnitCharacter* Creep = WU.Get())
			RemoveUnitFromAny(TM, Creep);
	Pair.PvECreeps.Reset();

	// 2) TM 비우기
	TM->ClearAll();

	// 3) 보드 스냅샷 복구 + 유닛 위치 재스냅
	if (APCPlayerBoard* HostPB = FindPlayerBoardBySeat(HostSeatIndex))
	{
		HostPB->ApplySnapshot(Pair.PvESnapShot);

		// 스냅샷대로 배열 복구 후, 액터를 보드 타일까지 이동
		for (int32 y=0; y<HostPB->Cols; ++y)
		{
			for (int32 x=0; x<HostPB->Rows; ++x)
			{
				const int32 i = HostPB->IndexOf(y,x);
				if (!HostPB->PlayerField.IsValidIndex(i)) continue;

				if (APCBaseUnitCharacter* U = HostPB->PlayerField[i].Unit)
				{
					const FVector Loc = HostPB->GetFieldWorldPos(y,x);
					const FRotator Rot(0.f, HostPB->GetActorRotation().Yaw, 0.f);
					U->SetActorLocationAndRotation(Loc, Rot, false, nullptr, ETeleportType::TeleportPhysics);
					U->ChangedOnTile(false);
				}
			}
		}
	}

	// 4) 언바인드 및 정리
	UnbindAllForPair(PairIndex);
	Pair.PvESnapShot = {};        // 스냅샷 해제
	Pair.ResetRuntime();
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

APCPlayerBoard* APCCombatManager::FindPlayerBoardBySeat(int32 SeatIndex) const
{
	if (APCPlayerState* PlayerState = FindPlayerStateBySeat(SeatIndex))
	{
		return PlayerState->GetPlayerBoard();
	}
	return nullptr;
}

void APCCombatManager::TakeFieldSnapShot(APCPlayerBoard* PlayerBoard, FBoardFieldSnapShot& Out)
{
	Out.Reset();
	if (!IsValid(PlayerBoard)) return;
	Out.PlayerBoard = PlayerBoard;

	const int32 Rows = PlayerBoard->Rows;
	const int32 Cols = PlayerBoard->Cols;
	for (int32 r = 0; r < Rows; ++r)
	{
		for (int32 c = 0; c < Cols; ++c)
		{
			const int32 i = PlayerBoard->IndexOf(c,r);
			if (!PlayerBoard->PlayerField.IsValidIndex(i)) continue;
			if (APCBaseUnitCharacter* Unit = PlayerBoard->PlayerField[i].Unit)
			{
				FCombatManager_FieldSlot Slot;
				Slot.Col = c;
				Slot.Row = r;
				Slot.Unit = Unit;
				Out.Field.Add(Slot);
			}
		}
	}
}

void APCCombatManager::RestoreFieldSnapShot(const FBoardFieldSnapShot& Snap)
{
	APCPlayerBoard* PlayerBoard = Snap.PlayerBoard.Get();
	if (!IsValid(PlayerBoard)) return;

	for (auto& Field : PlayerBoard->PlayerField)
	{
		if (Field.bIsField)
			Field.Unit = nullptr;
	}

	for (const auto& FieldSnap : Snap.Field)
	{
		const int32 i = PlayerBoard->IndexOf(FieldSnap.Col, FieldSnap.Row);
		if (!PlayerBoard->PlayerField.IsValidIndex(i)) continue;

		if (APCBaseUnitCharacter* Unit = FieldSnap.Unit.Get())
		{
			PlayerBoard->PlayerField[i].Unit = Unit;
			const FVector Loc = PlayerBoard->GetFieldWorldPos(FieldSnap.Col, FieldSnap.Row);
			const FRotator Rot(0.f, PlayerBoard->GetActorRotation().Yaw,0.f);
			Unit->SetActorLocationAndRotation(Loc,Rot,false,nullptr, ETeleportType::TeleportPhysics);
		}
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
	TArray<APCBaseUnitCharacter*> ToUnbind;
	for (auto It = UnitToPairIndex.CreateIterator(); It; ++It)
	{
		if (It.Value() == PairIndex)
		{
			const TWeakObjectPtr<APCBaseUnitCharacter> wUnit = It.Key();
			if (APCBaseUnitCharacter* Unit = wUnit.Get())
			{
				Unit->OnUnitDied.RemoveDynamic(this, &APCCombatManager::OnAnyUnitDied);
			}
			It.RemoveCurrent(); // 맵에서 제거
		}
	}
	
	if (Pairs.IsValidIndex(PairIndex))
	{
		Pairs[PairIndex].DeadUnits.Reset();
	}
	
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

	APCPlayerState* WinnerPlayerState = FindPlayerStateBySeat(bHostWon ? HostSeat : GuestSeat);
	APCPlayerState* LoserPlayerState = FindPlayerStateBySeat(bHostWon ? GuestSeat : HostSeat);
	if (!WinnerPlayerState || !LoserPlayerState) return;

	WinnerPlayerState->PlayerWin();
	LoserPlayerState->PlayerLose();

	const int32 StageIndex      = GetCurrentStageIndex();
	const int32 StageBaseDamage = GetStageBaseDamageFromDT(StageIndex);
	const int32 Alive           = bHostWon ? Pair.HostAlive : Pair.GuestAlive;
	const int32 Damage          = FMath::Max(1, StageBaseDamage + Alive);

 	FGameplayEventData DamageData;
	DamageData.EventTag       = DamageEventTag;
	DamageData.EventMagnitude = Damage;
	DamageData.Instigator     = WinnerPlayerState;
	DamageData.Target         = LoserPlayerState;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(WinnerPlayerState, DamageEventTag, DamageData);

	UnbindAllForPair(PairIndex);
	Pair.bRunning = false;

	const int32 WinnerSeat = bHostWon ? HostSeat : GuestSeat;
	const int32 LoserSeat  = bHostWon ? GuestSeat : HostSeat;
	OnCombatPairResult.Broadcast(WinnerSeat, LoserSeat, Pair.HostAlive, Pair.GuestAlive);
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
