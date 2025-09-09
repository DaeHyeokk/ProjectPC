// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFramework/GameMode/PCCombatGameMode.h"

#include "BaseGameplayTags.h"
#include "EngineUtils.h"
#include "INodeAndChannelMappings.h"
#include "Character/Unit/PCBaseUnitCharacter.h"
#include "Controller/Player/PCCombatPlayerController.h"
#include "EntitySystem/MovieSceneEntitySystemRunner.h"
#include "GameFramework/GameState.h"
#include "GameFramework/GameState/PCCombatGameState.h"
#include "GameFramework/HelpActor/PCCarouselRing.h"
#include "GameFramework/HelpActor/PCCombatBoard.h"
#include "GameFramework/HelpActor/PCCombatManager.h"
#include "GameFramework/HelpActor/Component/PCTileManager.h"
#include "GameFramework/PlayerState/PCPlayerState.h"
#include "GameFramework/WorldSubsystem/PCUnitGERegistrySubsystem.h"
#include "GameFramework/WorldSubsystem/PCUnitSpawnSubsystem.h"
#include "Kismet/GameplayStatics.h"


APCCombatGameMode::APCCombatGameMode()
{
	GameStateClass = APCCombatGameState::StaticClass();
	PlayerStateClass = APCPlayerState::StaticClass();
	PlayerControllerClass = APCCombatPlayerController::StaticClass();
	
	bUseSeamlessTravel = true;
}

void APCCombatGameMode::BeginPlay()
{
	Super::BeginPlay();
	BuildHelperActor();
	BuildStageData();

	GetWorldTimerManager().SetTimer(WaitAllPlayerController, this, &APCCombatGameMode::TryPlacePlayersAfterTravel, 0.1f, true, 0.15f);
	
	if (!GetWorld())
		return;
	
	if (!UnitGEDictionary)
		return;
	
	if (auto* UnitGERegistrySubsystem = GetWorld()->GetSubsystem<UPCUnitGERegistrySubsystem>())
	{
		FGameplayTagContainer PreloadGEClassTag;
		PreloadGEClassTag.AddTag(GameplayEffectTags::GE_Class_HealthChange);
		PreloadGEClassTag.AddTag(GameplayEffectTags::GE_Class_ManaChange);
		
		UnitGERegistrySubsystem->InitializeUnitGERegistry(UnitGEDictionary, PreloadGEClassTag);
	}
}

void APCCombatGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	if (!NewPlayer) return;

	auto* PS = Cast<APCPlayerState>(NewPlayer->PlayerState);
	if (!PS) return;

	bool Used[8] = {};
	if (GameState)
	{
		for (APlayerState* B : GameState->PlayerArray)
			if (auto* P = Cast<APCPlayerState>(B))
				if (P != PS && P->SeatIndex >= 0 && P->SeatIndex < 8)
					Used[P->SeatIndex] = true;
	}

	int32 Seat = 0; while (Seat < 8 && Used[Seat]) ++Seat;
	if (Seat >= 8) Seat = PS->GetPlayerId() % 8;

	PS->SeatIndex = Seat;
	PS->ForceNetUpdate();
}

void APCCombatGameMode::PostSeamlessTravel()
{
	Super::PostSeamlessTravel();
	if (AGameState* GS = GetGameState<AGameState>())
	{
		for (APlayerState* PS : GS->PlayerArray)
			if (APCPlayerState* PCPlayerState = Cast<APCPlayerState>(PS))
			{
				PCPlayerState->bIsReady = false;
				UE_LOG(LogTemp, Log, TEXT("PostSeamlessTravel SeatIndex=%d Name=%s"),
                					   PCPlayerState->SeatIndex, *PCPlayerState->GetPlayerName());
			}
	}

	GetWorldTimerManager().SetTimer(WaitAllPlayerController, this, &APCCombatGameMode::TryPlacePlayersAfterTravel,0.1f, true, 0.15f);
}

void APCCombatGameMode::AssignSeatInitial()
{
	if (!GameState)
		return;

	TArray<APCPlayerState*> Players;
	for (APlayerState* PlayerStateBase : GameState->PlayerArray)
	{
		if (APCPlayerState* PCPlayerState = Cast<APCPlayerState>(PlayerStateBase))
		{
			Players.Add(PCPlayerState);
		}
	}

	Players.Sort([](const APCPlayerState& A, const APCPlayerState& B)
	{
		return A.GetPlayerId() < B.GetPlayerId();
	});

	const int32 MaxSeats = 8;
	for (int32 i = 0; i < Players.Num(); ++i)
	{
		Players[i]->SeatIndex = ( i < MaxSeats) ? i : (i % MaxSeats);
		Players[i]->ForceNetUpdate();
	}
	
}

int32 APCCombatGameMode::GetTotalSeatSlots() const
{
	int32 ByRing = (CarouselRing ? FMath::Max(1, CarouselRing->NumSlots) : 0);
	int32 ByBoards = CombatBoard.Num();
	int32 Seats = (ByRing > 0 ? ByRing : (ByBoards > 0 ? ByBoards : 0));
	return Seats;
}

// 헬퍼 / 스케줄 빌드
void APCCombatGameMode::BuildHelperActor()
{
	CombatBoard.Reset();
	TArray<AActor*> OutBoard;
	UGameplayStatics::GetAllActorsOfClass(this, APCCombatBoard::StaticClass(), OutBoard);
	for (AActor* Actor : OutBoard)
	{
		CombatBoard.Add(Cast<APCCombatBoard>(Actor));
	}

	CombatBoard.Sort([](const APCCombatBoard& ABoard, const APCCombatBoard& BBoard)
	{
		return ABoard.BoardSeatIndex < BBoard.BoardSeatIndex;
	});

	if (APCCombatGameState* CombatGameState = GetGameState<APCCombatGameState>())
	{
		CombatGameState->BuildSeatToBoardMap(CombatBoard);
	}

	if (!CarouselRing)
	{
		for (TActorIterator<APCCarouselRing> It(GetWorld()); It; ++It)
		{
			CarouselRing = *It;
			break;
		}
	}
	
}

void APCCombatGameMode::BuildStageData()
{
	FlatRoundSteps.Reset();
	FlatStageIdx.Reset();
	FlatRoundIdx.Reset();
	FlatStepIdxInRound.Reset();

	if (ensure(StageData))
	{
		StageData->BuildFlattenedPhase(FlatRoundSteps, FlatStageIdx, FlatRoundIdx, FlatStepIdxInRound);
	}
	else
	{
		// 최소 Fallback: 한 스텝만
		FlatRoundSteps = { {EPCStageType::Start, 5.f} };
		FlatStageIdx = {0}; FlatRoundIdx = {0}; FlatStepIdxInRound = {0};
	}
}

void APCCombatGameMode::StartFromBeginning()
{
	Cursor = 0;
	BeginCurrentStep();
}

void APCCombatGameMode::AdvanceCursor()
{
	++Cursor;
}

void APCCombatGameMode::BeginCurrentStep()
{
	if (!GetCombatGameState() || !FlatRoundSteps.IsValidIndex(Cursor)) return;

	const FRoundStep& Step = FlatRoundSteps[Cursor];
	const float Duration = Step.DurationOverride > 0.f ? Step.DurationOverride
						: (StageData ? StageData->GetDefaultDuration(Step.StageType) : 30.f);

	// GameState 동기화
	FStageRuntimeState State;
	State.FloatIndex = Cursor;
	State.StageIdx = FlatStageIdx.IsValidIndex(Cursor) ? FlatStageIdx[Cursor] : 0;
	State.RoundIdx = FlatRoundIdx.IsValidIndex(Cursor) ? FlatRoundIdx[Cursor] : 0;
	State.StepIdxInRound = FlatStepIdxInRound.IsValidIndex(Cursor) ? FlatStepIdxInRound[Cursor] : 0;
	State.Stage = Step.StageType;
	State.Duration = Duration;
	State.ServerStartTime = NowServer();
	State.ServerEndTime = NowServer() + Duration;

	if (APCCombatGameState* PCGameState = GetCombatGameState())
	{
		PCGameState->SetStageRunTime(State);
	}

	const FString StageName = FString::Printf(TEXT("%d-%d"), State.StageIdx+1, State.RoundIdx+1);
	BroadcastStageToClients(Step.StageType, StageName, Duration);

	// 타이머
	GetWorldTimerManager().ClearTimer(RoundTimer);
	GetWorldTimerManager().SetTimer(RoundTimer, this, &APCCombatGameMode::EndCurrentStep, Duration, false);

	switch (Step.StageType)
	{
	case EPCStageType::Start : Step_Start(); break;
	case EPCStageType::Setup : Step_Setup(); break;
	case EPCStageType::Travel : Step_Travel(); break;
	case EPCStageType::Return : Step_Return(); break;
	case EPCStageType::PvP : Step_PvP(); break;
	case EPCStageType::CreepSpawn : Step_CreepSpawn(); break;
	case EPCStageType::PvE : Step_PvE(); break;
	case EPCStageType::Carousel : Step_Carousel(); break;
		default: break;
	}
}

void APCCombatGameMode::EndCurrentStep()
{
	AdvanceCursor();
	
	if (FlatRoundSteps.IsValidIndex(Cursor))
		BeginCurrentStep();

	
}

void APCCombatGameMode::Step_Start()
{
	PlaceAllPlayersOnCarousel();
}

void APCCombatGameMode::Step_Setup()
{
	
}

void APCCombatGameMode::Step_Travel()
{
	const FRoundStep* Next = PeekNextStep();
	if (!Next) return;

	switch (Next->StageType)
	{
	case EPCStageType::PvP:
		{
			if (APCCombatManager* PCCombatManager = GetCombatManager())
			{
				PCCombatManager->BuildRandomPairs();
				PCCombatManager->TravelPlayersForAllPairs(TravelCameraBlend);
				PCCombatManager->StartAllBattle();
			}
			break;
		}
	case EPCStageType::Carousel:
		{
			PlaceAllPlayersOnCarousel();
			SetCarouselCameraForAllPlayers();
			for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
			{
				if (APCCombatPlayerController* PCCombatPlayerController = Cast<APCCombatPlayerController>(*It))
				{
					PCCombatPlayerController->Client_HideWidget();
				}
			}
			break;
		}
	case EPCStageType::PvE:
		{
			
		}
		default:
		break;
	}
}

void APCCombatGameMode::Step_Return()
{
	const FRoundStep* Prev = PeekPrevStep();
	if (!Prev)
	{
		MovePlayersToBoardsAndCameraSet();
		return;
	}
	if (Prev->StageType == EPCStageType::PvP)
	{
		if (APCCombatManager* PCCombatManager = GetCombatManager())
		{
			PCCombatManager->FinishAllBattle();
			PCCombatManager->ReturnPlayersForAllPairs(ReturnCameraBlend);
		}
	}
	else if (Prev->StageType == EPCStageType::Carousel)
	{
		MovePlayersToBoardsAndCameraSet();
	}
	else if (Prev->StageType == EPCStageType::Start)
	{
		MovePlayersToBoardsAndCameraSet();
		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			if (APCCombatPlayerController* PCCombatPlayerController = Cast<APCCombatPlayerController>(*It))
			{
				PCCombatPlayerController->Client_ShowWidget();
				if (APCCombatGameState* PCGameState = GetCombatGameState())
				{
					if (APCPlayerState* PCPlayerState = PCCombatPlayerController->GetPlayerState<APCPlayerState>())
					{
						APCCombatBoard* InCombatBoard = PCGameState->GetBoardBySeat(PCPlayerState->SeatIndex);
						const FVector UnitSpawnLoc = InCombatBoard->GetTileWorldLocation(0,1) + FVector(0,0,20);
						const FVector UnitSpawnLoc2 = InCombatBoard->GetTileWorldLocation(1,1) + FVector(0,0,20);
						const FTransform SpawnTransform2(FRotator::ZeroRotator, UnitSpawnLoc2, FVector::OneVector);
						const FTransform SpawnTransform(FRotator::ZeroRotator, UnitSpawnLoc, FVector::OneVector);
						if (UPCUnitSpawnSubsystem* SpawnSubsystem = GetWorld()->GetSubsystem<UPCUnitSpawnSubsystem>())
						{
							APCBaseUnitCharacter* Unit = SpawnSubsystem->SpawnUnitByTag(UnitGameplayTags::Unit_Type_Hero_Sparrow, SpawnTransform, PCPlayerState->SeatIndex);
							InCombatBoard->TileManager->PlaceUnitOnField(0,1, Unit);
							APCBaseUnitCharacter* Unit2 = SpawnSubsystem->SpawnUnitByTag(UnitGameplayTags::Unit_Type_Hero_Sparrow, SpawnTransform2, PCPlayerState->SeatIndex);
							InCombatBoard->TileManager->PlaceUnitOnField(1,1, Unit2);
							InCombatBoard->TileManager->DebugLogField(true,true, TEXT("TileManager"));
						}
					}
							
				}
				
			}
		}
		
	}
	else
	{
		
	}
}

void APCCombatGameMode::Step_PvP()
{
	
}

void APCCombatGameMode::Step_PvE()
{
	if (APCCombatGameState* PCGameState = GetCombatGameState())
	{
		PCGameState->SetGameStateTag(GameStateTags::Game_State_Combat_Active);
	}
}


void APCCombatGameMode::Step_CreepSpawn()
{
	if (APCCombatManager* PCCombatManager = GetCombatManager())
	{
		PCCombatManager->BuildRandomPairs();
		PCCombatManager->TravelPlayersForAllPairs(TravelCameraBlend);
		PCCombatManager->StartAllBattle();
	}
}

void APCCombatGameMode::Step_Carousel()
{
}

void APCCombatGameMode::InitializeHomeBoardsForPlayers()
{
	if ( CombatBoard.Num() == 0)
		return;
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APCCombatPlayerController* PCCombatPlayerController = Cast<APCCombatPlayerController>(*It);
		if (!PCCombatPlayerController) continue;
		APCPlayerState* PCPlayerState = PCCombatPlayerController->GetPlayerState<APCPlayerState>();
		if (!PCPlayerState) continue;

		const int32 BoardIdx = ResolveBoardIndex(PCPlayerState);

		PCCombatPlayerController->ClientSetHomeBoardIndex(BoardIdx);
	}
}

void APCCombatGameMode::TryPlacePlayersAfterTravel()
{
	AGameStateBase* GameStateBase = GameState;
	if (!GameStateBase) return;

	if (GameStateBase->PlayerArray.Num() <= 0)
		return;

	int32 NumPlayerController = 0;
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		++NumPlayerController;
	}
	if (NumPlayerController <= 0)
		return;

	InitializeHomeBoardsForPlayers();
	
	GetWorldTimerManager().ClearTimer(WaitAllPlayerController);
	
	UE_LOG(LogTemp, Log, TEXT("[GM] Placed players on carousel after travel. PCs=%d, Players=%d"),
		NumPlayerController, GameStateBase->PlayerArray.Num());

	StartFromBeginning();
}

void APCCombatGameMode::PlaceAllPlayersOnCarousel()
{
	if (!CarouselRing)
		return;

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PlayerController = It->Get();
		if (!PlayerController) continue;

		APCPlayerState* PCPlayerState = PlayerController->GetPlayerState<APCPlayerState>();
		if (!PCPlayerState) continue;

		const int32 Seat = FMath::Max(0, PCPlayerState->SeatIndex);
		const FTransform Transform = CarouselRing->GetSlotTransformWorld(Seat);
		
		// Pawn 보장
		if (APawn* PlayerPawn = PlayerController->GetPawn())
		{
			PlayerPawn->TeleportTo(Transform.GetLocation(), PlayerPawn->GetActorRotation(),false,true);
		}

		GetWorldTimerManager().SetTimer(CameraSetupTimer, this, &APCCombatGameMode::SetCarouselCameraForAllPlayers, 0.1f, true, 0.15f);
	}
}

void APCCombatGameMode::MovePlayersToBoardsAndCameraSet()
{
	if (CombatBoard.Num() == 0)
		return;
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APCCombatPlayerController* PlayerController = Cast<APCCombatPlayerController>(*It);
		if (!PlayerController) continue;

		APCPlayerState* PlayerState = PlayerController->GetPlayerState<APCPlayerState>();
		if (!PlayerState) continue;

		const int32 BoardIdx = ResolveBoardIndex(PlayerState);
		APCCombatBoard* Board = CombatBoard.IsValidIndex(BoardIdx) ? CombatBoard[BoardIdx] : nullptr;
		if (!Board) continue;

		if (APawn* Pawn = PlayerState->GetPawn())
		{
			const FTransform Seat = Board->GetPlayerSeatTransform();
			Pawn->TeleportTo(Seat.GetLocation(), Pawn->GetActorRotation(), false, true);
		}
		
		PlayerController->ClientFocusBoardBySeatIndex(BoardIdx, false, ShopFocusBlend);
	}
}

void APCCombatGameMode::SetCarouselCameraForAllPlayers()
{
	if (!CarouselRing || !IsValid(CarouselRing))
		return;

	int32 CameraSetCount = 0;
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (APCCombatPlayerController* PCCombatController = Cast<APCCombatPlayerController>(It->Get()))
		{
			UE_LOG(LogTemp, Warning, TEXT("Setting carousel camera for player: %s"), 
		   *PCCombatController->GetName());
			PCCombatController->ClientCameraSetCarousel(CarouselRing, CentralCameraBlend);
			++CameraSetCount;
		}
	}

	GetWorldTimerManager().ClearTimer(CameraSetupTimer);

	UE_LOG(LogTemp, Log, TEXT("Set carousel camera for %d players"), CameraSetCount);
	
}

int32 APCCombatGameMode::ResolveBoardIndex(const APCPlayerState* PlayerState) const
{
	if (!PlayerState || CombatBoard.Num() == 0)
		return 0;
	return FMath::Clamp(PlayerState->SeatIndex % CombatBoard.Num(), 0, CombatBoard.Num()-1);
}

void APCCombatGameMode::BroadcastStageToClients(EPCStageType Stage, const FString& StageName, float Seconds)
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		if (auto* PlayerController = Cast<APCCombatPlayerController>(*It))
			PlayerController->ClientStageChanged(Stage, StageName, Seconds);
}

const FRoundStep* APCCombatGameMode::PeekPrevStep() const
{
	const int32 Prev = Cursor - 1;
	return FlatRoundSteps.IsValidIndex(Prev) ? &FlatRoundSteps[Prev] : nullptr;
}

const FRoundStep* APCCombatGameMode::PeekNextStep() const
{
	const int32 Next = Cursor + 1;
	return FlatRoundSteps.IsValidIndex(Next) ? &FlatRoundSteps[Next] : nullptr;
}

APCCombatGameState* APCCombatGameMode::GetCombatGameState() const
{
	return GetGameState<APCCombatGameState>();
}

APCCombatManager* APCCombatGameMode::GetCombatManager()
{
	if (CombatManager.IsValid())
		return CombatManager.Get();
	for (TActorIterator<APCCombatManager> It(GetWorld()); It; ++It)
	{
		CombatManager = *It;
		return CombatManager.Get();
	}
	return nullptr;
}

APCPlayerState* APCCombatGameMode::FindPlayerStateBySeat(int32 SeatIdx)
{
	if (AGameStateBase* GS = GameState)
	{
		for (APlayerState* PS : GS->PlayerArray)
		{
			if (auto* P = Cast<APCPlayerState>(PS))
			{
				if (P->SeatIndex == SeatIdx)
					return P;
			}
		}
	}
	return nullptr;
}
