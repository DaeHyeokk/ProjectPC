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
	PlayerControllerClass = APCCombatPlayerController::StaticClass();
}

void APCCombatGameMode::BeginPlay()
{
	Super::BeginPlay();
	BuildHelperActor();
	BuildStageData();
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

	if (!CarouseRing)
	{
		for (TActorIterator<APCCarouselRing> It(GetWorld()); It; ++It)
		{
			CarouseRing = *It;
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

	const FString StageName = FString::Printf(TEXT("%d-%d"), GetCombatGameState()->StageIdx+1, GetCombatGameState()->StepIdxInRound+1);
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
	if (FlatRoundSteps.IsValidIndex(Cursor))
		BeginCurrentStep();
}



void APCCombatGameMode::Step_Start()
{
	PlaceAllPlayersOnCarousel();

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (auto* PlayerController = Cast<APCCombatPlayerController>(*It))
		{
			PlayerController->ClientCameraSetByActorName(TEXT("CarouselRing"), CentralCameraBlend);
		}
	}
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

void APCCombatGameMode::PlaceAllPlayersOnCarousel()
{
	if (!CarouseRing)
		return;

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		auto* PCPlayerController = Cast<APCCombatPlayerController>(*It);
		if (!PCPlayerController) continue;

		if (auto* PlayerState = PCPlayerController->GetPlayerState<APCPlayerState>())
		{
			if (APawn* Pawn = PlayerState->GetPawn())
			{
				const int32 Seat = FMath::Max(0, PlayerState->SeatIndex);
				const FTransform Transform = CarouseRing->GetSlotTransformWorld(Seat);
				Pawn->TeleportTo(Transform.GetLocation(), Pawn->GetActorRotation(), false, true);
			}
		}		
	}
}

void APCCombatGameMode::MovePlayersToBoardsAndCameraSet()
{
	if (CombatBoard.Num() == 0)
		return;
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		auto* PlayerController = Cast<APCCombatPlayerController>(*It);
		if (!PlayerController) continue;

		auto* PlayerState = PlayerController->GetPlayerState<APCPlayerState>();
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

void APCCombatGameMode::PostSeamlessTravel()
{
	Super::PostSeamlessTravel();
	if (auto* GS = GetGameState<AGameState>())
	{
		for (APlayerState* PS : GS->PlayerArray)
			if (auto* PCPlayerState = Cast<APCPlayerState>(PS))
				PCPlayerState->bIsReady = false;
	}
}
