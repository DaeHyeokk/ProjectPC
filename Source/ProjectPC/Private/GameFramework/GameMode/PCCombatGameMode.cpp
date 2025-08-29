// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFramework/GameMode/PCCombatGameMode.h"

#include "BaseGameplayTags.h"
#include "EngineUtils.h"
#include "INodeAndChannelMappings.h"
#include "Controller/Player/PCCombatPlayerController.h"
#include "GameFramework/GameState.h"
#include "GameFramework/GameState/PCCombatGameState.h"
#include "GameFramework/HelpActor/PCCarouselRing.h"
#include "GameFramework/HelpActor/PCCombatBoard.h"
#include "GameFramework/PlayerState/PCPlayerState.h"
#include "GameFramework/GameInstanceSubsystem/PCUnitGERegistrySubsystem.h"
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
	StartFromBeginning();

	if (!UnitGEDictionary)
		return;
	
	const UGameInstance* GI = GetWorld()->GetGameInstance();
	if (auto* UnitGERegistrySubsystem = GI->GetSubsystem<UPCUnitGERegistrySubsystem>())
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

void APCCombatGameMode::AssignSeatInitial(bool bForceReassign)
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

	const int32 TotalSeats = GetTotalSeatSlots();

	if (bForceReassign)
	{
		for (APCPlayerState* PCPlayerState : Players)
		{
			PCPlayerState->SeatIndex = -1;
		}
	}

	TBitArray<> Used(false, TotalSeats);
	for (APCPlayerState* PCPlayerState : Players)
	{
		if (PCPlayerState->SeatIndex >= 0 && PCPlayerState->SeatIndex < TotalSeats)
		{
			Used[PCPlayerState->SeatIndex] = true;
		}
	}

	int32 Next = 0;
	auto NextFree = [&]()
	{
		while (Next < TotalSeats && Used[Next]) ++Next;
		return (Next < TotalSeats) ? Next : INDEX_NONE;
	};

	for (APCPlayerState* PCPlayerState : Players)
	{
		if (PCPlayerState->SeatIndex >0 && PCPlayerState->SeatIndex < TotalSeats) continue;

		int32 Free = NextFree();
		if (Free == INDEX_NONE)
		{
			const int32 IndexInArray = Players.IndexOfByKey(PCPlayerState);
			Free = (TotalSeats > 0 ? IndexInArray % TotalSeats : 0);
		}
		else
		{
			Used[Free] = true;
		}

		PCPlayerState->SeatIndex = Free;
		PCPlayerState->ForceNetUpdate();
	}
}

void APCCombatGameMode::AssignSeatIfNeeded(class APCPlayerState* PCPlayerState)
{
	if (!PCPlayerState)
		return;
	const int32 TotalSeats = GetTotalSeatSlots();
	if (PCPlayerState->SeatIndex >= 0 && PCPlayerState->SeatIndex < TotalSeats)
		return;
	int32 FreeSeat = FindNextFreeSeat(TotalSeats);
	if (FreeSeat == INDEX_NONE)
	{
		const int32 IndexInArray = GameState ? GameState->PlayerArray.IndexOfByKey(PCPlayerState) : 0;
		FreeSeat = (TotalSeats > 0 ? IndexInArray % TotalSeats : 0);
	}
	PCPlayerState->SeatIndex = FreeSeat;
	PCPlayerState->ForceNetUpdate();
}

int32 APCCombatGameMode::GetTotalSeatSlots() const
{
	int32 ByRing = (CarouselRing ? FMath::Max(1, CarouselRing->NumSlots) : 0);
	int32 ByBoards = CombatBoard.Num();
	int32 Seats = (ByRing > 0 ? ByRing : (ByBoards > 0 ? ByBoards : 0));
	return Seats;
}

int32 APCCombatGameMode::FindNextFreeSeat(int32 TotalSeats) const
{
	if (!GameState || TotalSeats <= 0 )
		return INDEX_NONE;
	TBitArray<> Used(false, TotalSeats);
	for (APlayerState* PlayerStateBase : GameState->PlayerArray)
	{
		if (APCPlayerState* PCPlayerState = Cast<APCPlayerState>(PlayerStateBase))
		{
			Used[PCPlayerState->SeatIndex] = true;
		}
	}

	for (int32 i = 0; i < TotalSeats; ++i)
	{
		if (!Used[i])
			return i;
	}

	return INDEX_NONE;
}

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

	if (!StageData)
	{
		FlatRoundSteps = {{EPCStageType::Start, 0.f},
			{EPCStageType::Shop, 0.f},
			{EPCStageType::PvP, 0.f}};
		FlatStageIdx = {0,0,0};
		FlatRoundIdx = {0,1,2};
		FlatStepIdxInRound = {0,0,0};
		return;
	}
	StageData->BuildFlattenedPhase(FlatRoundSteps, FlatStageIdx, FlatRoundIdx, FlatStepIdxInRound);
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
	GetCombatGameState()->FloatIndex = Cursor;
	GetCombatGameState()->StageIdx = FlatStageIdx.IsValidIndex(Cursor) ? FlatStageIdx[Cursor] : 0;
	GetCombatGameState()->RoundIdx = FlatRoundIdx.IsValidIndex(Cursor) ? FlatRoundIdx[Cursor] : 0;
	GetCombatGameState()->StepIdxInRound = FlatStepIdxInRound.IsValidIndex(Cursor) ? FlatStepIdxInRound[Cursor] : 0;
	GetCombatGameState()->CurrentStage = Step.StageType;
	GetCombatGameState()->StageDuration = Duration;
	GetCombatGameState()->StageEndTime_Server = NowServer() + Duration;

	const FString StageName = FString::Printf(TEXT("%d-%d"), GetCombatGameState()->StageIdx+1, GetCombatGameState()->RoundIdx+1);
	BroadcastStageToClients(Step.StageType, StageName, Duration);

	// 타이머
	GetWorldTimerManager().ClearTimer(RoundTimer);
	GetWorldTimerManager().SetTimer(RoundTimer, this, &APCCombatGameMode::EndCurrentStep, Duration, false);

	switch (Step.StageType)
	{
	case EPCStageType::Start : Step_Start(); break;
	case EPCStageType::Shop : Step_Shop(); break;
	case EPCStageType::PvP : Step_PvP(); break;
	case EPCStageType::PvE : Step_PvE(); break;
	case EPCStageType::Carousel : Step_Carousel(); break;
		default: break;
	}
}

void APCCombatGameMode::EndCurrentStep()
{
	AdvanceCursor();
	MovePlayersToBoardsAndCameraSet();
	
	if (FlatRoundSteps.IsValidIndex(Cursor))
		BeginCurrentStep();

	
}



void APCCombatGameMode::Step_Start()
{
	// for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	// {
	// 	if (APCCombatPlayerController* PlayerController = Cast<APCCombatPlayerController>(*It))
	// 	{
	// 		PlayerController->ClientCameraSetByActorName(TEXT("CarouselRing"), CentralCameraBlend);
	// 	}
	// }
}

void APCCombatGameMode::Step_Shop()
{
}

void APCCombatGameMode::Step_PvP()
{
}

void APCCombatGameMode::Step_PvE()
{
}

void APCCombatGameMode::Step_Carousel()
{
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

	AssignSeatInitial(true);

	PlaceAllPlayersOnCarousel();

	GetWorldTimerManager().ClearTimer(WaitAllPlayerController);

	
	UE_LOG(LogTemp, Log, TEXT("[GM] Placed players on carousel after travel. PCs=%d, Players=%d"),
		NumPlayerController, GameStateBase->PlayerArray.Num());
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

		PlayerController->ClientCameraSet(BoardIdx, ShopFocusBlend);
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

