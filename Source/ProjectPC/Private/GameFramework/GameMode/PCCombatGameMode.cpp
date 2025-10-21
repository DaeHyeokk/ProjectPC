// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFramework/GameMode/PCCombatGameMode.h"

#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"

#include "BaseGameplayTags.h"
#include "AbilitySystem/Player/AttributeSet/PCPlayerAttributeSet.h"
#include "Character/Player/PCPlayerCharacter.h"
#include "Controller/Player/PCCombatPlayerController.h"
#include "EntitySystem/MovieSceneEntitySystemRunner.h"
#include "GameFramework/GameInstanceSubsystem/ProfileSubsystem.h"
#include "GameFramework/GameState/PCCombatGameState.h"
#include "GameFramework/HelpActor/PCCarouselRing.h"
#include "GameFramework/HelpActor/PCCombatBoard.h"
#include "GameFramework/HelpActor/PCCombatManager.h"
#include "GameFramework/HelpActor/PCPlayerBoard.h"
#include "GameFramework/HelpActor/Component/PCTileManager.h"
#include "GameFramework/PlayerState/PCPlayerState.h"
#include "GameFramework/WorldSubsystem/PCUnitGERegistrySubsystem.h"
#include "GameFramework/WorldSubsystem/PCUnitSpawnSubsystem.h"
#include "Shop/PCShopManager.h"


APCCombatGameMode::APCCombatGameMode()
{
	GameStateClass = APCCombatGameState::StaticClass();
	PlayerStateClass = APCPlayerState::StaticClass();
	PlayerControllerClass = APCCombatPlayerController::StaticClass();
}

void APCCombatGameMode::BeginPlay()
{
	Super::BeginPlay();
	BuildHelperActor();
	BuildStageData();
	
			
	if (!GetWorld())
		return;
	
	if (!UnitGEDictionary)
		return;
	
	if (auto* UnitGERegistrySubsystem = GetWorld()->GetSubsystem<UPCUnitGERegistrySubsystem>())
	{
		FGameplayTagContainer PreloadGEClassTag;
		PreloadGEClassTag.AddTag(GameplayEffectTags::GE_Class_Unit_Damage);
		PreloadGEClassTag.AddTag(GameplayEffectTags::GE_Class_Unit_Heal);
		PreloadGEClassTag.AddTag(GameplayEffectTags::GE_Class_Unit_ManaChange);
		
		UnitGERegistrySubsystem->InitializeUnitGERegistry(UnitGEDictionary, PreloadGEClassTag);
	}

	// GetWorldTimerManager().SetTimer(WaitAllPlayerController, this, &APCCombatGameMode::TryPlacePlayersAfterTravel,2.f, true, 0.f);
}

void APCCombatGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	APCPlayerState* PS = NewPlayer ? Cast<APCPlayerState>(NewPlayer->PlayerState) : nullptr;
	if (!PS) return;
    
	// 사용중인 좌석 체크
	TArray<bool> UsedSeats;
	UsedSeats.Init(false, 8);
    
	for (APlayerState* PlayerState : GameState->PlayerArray)
	{
		if (APCPlayerState* OtherPS = Cast<APCPlayerState>(PlayerState))
		{			
			if (OtherPS != PS && OtherPS->SeatIndex >= 0 && OtherPS->SeatIndex < 8)
			{
				UsedSeats[OtherPS->SeatIndex] = true;
			}
		}
	}
    
	// 빈 좌석 찾기
	int32 SeatIndex = 0;
	for (int32 i = 0; i < 8; i++)
	{
		if (!UsedSeats[i])
		{
			SeatIndex = i;
			break;
		}
	}
	
	PS->SeatIndex = SeatIndex;
	PS->ForceNetUpdate();
	
	if (auto* PC = Cast<APCCombatPlayerController>(NewPlayer))
	{
		PC->Client_RequestIdentity();
	}
	
	OnOnePlayerArrived();
}

int32 APCCombatGameMode::GetTotalSeatSlots() const
{
	int32 ByRing = (CarouselRing ? FMath::Max(1, CarouselRing->PlayerNumSlots) : 0);
	int32 ByBoards = CombatBoard.Num();
	int32 Seats = (ByRing > 0 ? ByRing : (ByBoards > 0 ? ByBoards : 0));
	return Seats;
}

void APCCombatGameMode::BindPlayerAttribute()
{
	if (APCCombatGameState* CombatGameState = GetCombatGameState())
	{
		CombatGameState->BindAllPlayerHP();
	}
}

void APCCombatGameMode::BindPlayerMainHuD()
{
	APCCombatGameState* CombatGameState = GetCombatGameState();
	if (!CombatGameState) return;
	
	for (int32 i = 0; i < CombatGameState->Leaderboard.Num(); ++i)
	{
		FString UserId = CombatGameState->Leaderboard[i].LocalUserId;
		UE_LOG(LogTemp, Warning, TEXT("LeaderBoard idx : %d LocalUserId : %s"), i, *UserId );
	}
	
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (APCCombatPlayerController* PCPlayerController = Cast<APCCombatPlayerController>(*It))
		{
			PCPlayerController->LoadMainWidget();
		}
	}
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

	TArray<FRoundStep> Steps; TArray<int32> SIdx, RIdx, KIdx;
	TArray<FGameplayTag> RoundMajorFlat;
	TArray<FGameplayTag>  PvESubTagFlat;

	StageData->BuildFlattenedPhase(Steps, SIdx, RIdx, KIdx, RoundMajorFlat, PvESubTagFlat);

	// 네가 쓰는 플랫 스텝/인덱스 유지
	FlatRoundSteps     = Steps;
	FlatStageIdx       = SIdx;
	FlatRoundIdx       = RIdx;
	FlatStepIdxInRound = KIdx;
	
	TArray<int32> Counts;

	int32 MaxStage = -1;
	for (int32 v : SIdx)
	{
		MaxStage = FMath::Max(MaxStage, v);
	}
	Counts.Init(0, MaxStage + 1);

	for (int32 i = 0; i < SIdx.Num() && i < KIdx.Num(); ++i)
	{
		const int32 Stage = SIdx[i];
		const int32 StepInRound = KIdx[i];
		if (Stage >= 0 && Stage < Counts.Num() && StepInRound == 0)
		{
			++Counts[Stage];
		}
	}
	

	if (APCCombatGameState* GS = GetGameState<APCCombatGameState>())
	{
		if (HasAuthority())
		{
			GS->SetRoundsPerStage(Counts);
			GS->SetRoundMajorsFlat(RoundMajorFlat);
			GS->RoundPvETagFlat = PvESubTagFlat;
			GS->ForceNetUpdate();
		}
	}
}

void APCCombatGameMode::StartFromBeginning()
{
	GetWorldTimerManager().ClearTimer(StartTimer);
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
	const int32 Stage = FlatStageIdx.IsValidIndex(Cursor) ? FlatStageIdx[Cursor] : 0;
	const int32 Round = FlatRoundIdx.IsValidIndex(Cursor) ? FlatRoundIdx[Cursor] : 0;
	const bool NotReward = (Stage == 1.f && Round == 2.f);

	if (APCCombatGameState* PCCombatGameState = GetCombatGameState())
	{
		PCCombatGameState->SetGameStateTag(GameStateTags::Game_State_NonCombat);
	}

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (auto* PCPlayerController = Cast<APCCombatPlayerController>(*It))
		{
			PCPlayerController->Client_ShowWidget();
			PCPlayerController->Server_ShopRefresh(0);

			if (APCPlayerState* PCPlayerState = PCPlayerController->GetPlayerState<APCPlayerState>())
			{
				if (!NotReward)
				{
					PCPlayerState->ApplyRoundReward();
				}
			}
			PCPlayerController->Client_ShowWidget();
			
		}
	}
}

void APCCombatGameMode::Step_Travel()
{
	const int32 Stage = FlatStageIdx.IsValidIndex(Cursor) ? FlatStageIdx[Cursor] : 0;
	const FRoundStep* Next = PeekNextStep();
	if (!Next) return;

	switch (Next->StageType)
	{
	case EPCStageType::PvP:
		{
			if (APCCombatGameState* PCGameState = GetCombatGameState())
			{
				PCGameState->SetGameStateTag(GameStateTags::Game_State_Combat_Preparation);
			}
			
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
			if (CarouselRing)
			{
				CarouselRing->Server_StartCarousel(0.f,15.f);
				CarouselRing->SpawnPickups(Stage);
			}
			
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
			if (APCCombatGameState* PCGameState = GetCombatGameState())
			{
				PCGameState->SetGameStateTag(GameStateTags::Game_State_Combat_Preparation);
			}
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

		if (APCCombatGameState* PCGameState = GetCombatGameState())
		{
			PCGameState->SetGameStateTag(GameStateTags::Game_State_Combat_End);
		}
		
		if (APCCombatManager* PCCombatManager = GetCombatManager())
		{
			PCCombatManager->FinishAllBattle();
			PCCombatManager->ReturnPlayersForAllPairs(ReturnCameraBlend);
		}
	}
	else if (Prev->StageType == EPCStageType::Carousel)
	{
		MovePlayersToBoardsAndCameraSet();
		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			if (APCCombatPlayerController* PCCombatPlayerController = Cast<APCCombatPlayerController>(*It))
			{
				PCCombatPlayerController->Client_ShowWidget();
			}
		}
		PlaceAllPlayersPickUpUnit();
	}
	else if (Prev->StageType == EPCStageType::Start)
	{
		MovePlayersToBoardsAndCameraSet();
		PlayerStartUnitSpawn();
	}
	else if (Prev->StageType == EPCStageType::PvE)
	{
		if (APCCombatGameState* PCGameState = GetCombatGameState())
		{
			PCGameState->SetGameStateTag(GameStateTags::Game_State_Combat_End);

			if (APCCombatManager* PCCombatManager = GetCombatManager())
			{
				PCCombatManager->FinishAllPve();
			}
		}
	}
}

void APCCombatGameMode::Step_PvP()
{
	if (APCCombatGameState* PCGameState = GetCombatGameState())
	{
		PCGameState->SetGameStateTag(GameStateTags::Game_State_Combat_Active);
	}
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
	APCCombatGameState* PCGameState = GetCombatGameState();
	if (!PCGameState) return;

	PCGameState->SetGameStateTag(GameStateTags::Game_State_Combat_Preparation);

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (APCCombatPlayerController* PCPlayerController = Cast<APCCombatPlayerController>(*It))
		{
			if (APCPlayerState* PCPlayerState = PCPlayerController->GetPlayerState<APCPlayerState>())
			{
				if (APCCombatManager* PCCombatManager = GetCombatManager())
				{
					PCCombatManager->StartPvEBattleForSeat(PCPlayerState->SeatIndex);
				}
			}
		}
	}
}

void APCCombatGameMode::Step_Carousel()
{
	if (!CarouselRing) return;

	BuildCarouselWavesByHP(CarouselWaves);
	if (CarouselWaves.Num() == 0)
	{
		FinishCarouselRound();
		return;
	}

	const int32 WaveCount = CarouselWaves.Num();
	const float TotalDuration = (WaveCount > 1) ? (WaveCount-1)*5.f + 8.f : 8.f;
	
	if (APCCombatGameState* GS = GetCombatGameState())
	{
		FStageRuntimeState S;
		S.FloatIndex       = Cursor;
		S.StageIdx         = FlatStageIdx.IsValidIndex(Cursor) ? FlatStageIdx[Cursor] : 0;
		S.RoundIdx         = FlatRoundIdx.IsValidIndex(Cursor) ? FlatRoundIdx[Cursor] : 0;
		S.StepIdxInRound   = FlatStepIdxInRound.IsValidIndex(Cursor) ? FlatStepIdxInRound[Cursor] : 0;
		S.Stage            = EPCStageType::Carousel;
		S.Duration         = TotalDuration;
		S.ServerStartTime  = NowServer();
		S.ServerEndTime    = S.ServerStartTime + TotalDuration;
		GS->SetStageRunTime(S);
	}

	// 기존 RoundTimer(상위 BeginCurrentStep에서 잡힌 것) 무시하고, 우리 스케줄로 교체
	GetWorldTimerManager().ClearTimer(RoundTimer);
	GetWorldTimerManager().SetTimer(RoundTimer, this, &APCCombatGameMode::EndCurrentStep, TotalDuration, false);

	// 웨이브 진행 시작
	StartCarouselWaves();
}

void APCCombatGameMode::PlayerStartUnitSpawn()
{
	if (APCCombatGameState* PCGameState = GetCombatGameState())
	{
		int32 SpawnIndex = 0;
		auto ShopManager = PCGameState->GetShopManager();
		TArray<FGameplayTag> SpawnTag = ShopManager->GetCarouselUnitTags(1);
		PCGameState->SetGameStateTag(GameStateTags::Game_State_NonCombat);

		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			if (APCCombatPlayerController* PCCombatPlayerController = Cast<APCCombatPlayerController>(*It))
			{
				if (APCPlayerState* PCPlayerState = PCCombatPlayerController->GetPlayerState<APCPlayerState>())
				{
					PCPlayerState->UnitSpawn(SpawnTag[SpawnIndex]);
					++SpawnIndex;
				}
			}
		}
	}
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
		APCCombatBoard* Board = CombatBoard.IsValidIndex(BoardIdx) ? CombatBoard[BoardIdx] : nullptr;
		if (!Board) continue;
		
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

	GetWorldTimerManager().ClearTimer(WaitAllPlayerController);

	AssignSeatDeterministicOnce();

	CollectPlayerBoards();
	
	GetWorldTimerManager().SetTimer(ThWaitReady, this, &APCCombatGameMode::StartWhenReady,0.25f, true,0.0f);
	
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
		const FTransform Transform = CarouselRing->GetPlayerSlotTransformWorld(Seat);
		
		if (APCPlayerCharacter* PlayerPawn = PlayerController->GetPawn<APCPlayerCharacter>())
		{
			PlayerPawn->TeleportTo(Transform.GetLocation(), PlayerPawn->GetActorRotation(),false,true);
		}
	}
	SetCarouselCameraForAllPlayers();
}

void APCCombatGameMode::PlaceAllPlayersPickUpUnit()
{
	if (!CarouselRing)
		return;

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PlayerController = It->Get();
		if (!PlayerController) continue;

		APCPlayerState* PCPlayerState = PlayerController->GetPlayerState<APCPlayerState>();
		if (!PCPlayerState) continue;
		
		if (APCPlayerCharacter* PlayerPawn = PlayerController->GetPawn<APCPlayerCharacter>())
		{
			PlayerPawn->CarouselUnitToSpawn();
			
		}
	}

	CarouselRing->Server_FinishCarousel();
	CarouselRing->Multicast_OpenAllGates(false);
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
		
		PlayerController->ClientFocusBoardBySeatIndex(BoardIdx, false, 0);
	}
}

void APCCombatGameMode::SetCarouselCameraForAllPlayers()
{
	if (!CarouselRing || !IsValid(CarouselRing))
		return;

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (APCCombatPlayerController* PCCombatController = Cast<APCCombatPlayerController>(It->Get()))
		{
			if (APCPlayerState* PCPlayerState = PCCombatController->GetPlayerState<APCPlayerState>())
			{
				PCCombatController->ClientCameraSetCarousel(CarouselRing,PCPlayerState->SeatIndex, 0);
			}
		}
	}
		
}

int32 APCCombatGameMode::ResolveBoardIndex(const APCPlayerState* PlayerState) const
{
	if (!PlayerState || CombatBoard.Num() == 0)
		return 0;
	return FMath::Clamp(PlayerState->SeatIndex % CombatBoard.Num(), 0, CombatBoard.Num()-1);
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

void APCCombatGameMode::CollectPlayerBoards()
{
	AllPlayerBoards.Reset();
	SeatToPlayerBoard.Reset();

	TArray<AActor*> FoundActor;
	UGameplayStatics::GetAllActorsOfClass(this, APCPlayerBoard::StaticClass(), FoundActor);

	for (AActor* Actor : FoundActor)
	{
		if (auto* PlayerBoard = Cast<APCPlayerBoard>(Actor))
		{
			AllPlayerBoards.Add(PlayerBoard);
			if (PlayerBoard->PlayerIndex >= 0)
			{
				SeatToPlayerBoard.FindOrAdd(PlayerBoard->PlayerIndex) = PlayerBoard;
			}
		}
	}
	
	UE_LOG(LogTemp, Log, TEXT("[GM] PlayerBoards collected: %d"), AllPlayerBoards.Num());
}

void APCCombatGameMode::BindPlayerBoardsToPlayerStates()
{
	if (!GameState) return;

	for (APlayerState* PSBase : GameState->PlayerArray)
	{
		auto* PCPS = Cast<APCPlayerState>(PSBase);
		if (!PCPS) continue;

		APCPlayerBoard* PlayerBoard = FindPlayerBoardBySeat(PCPS->SeatIndex);
		if (!IsValid(PlayerBoard))
		{
			continue;
		}
				
		// 보드에 내 Seat 기록(검색용)
		PCPS->SetPlayerBoard(PlayerBoard);
		ForceNetUpdate();

		PlayerBoard->PlayerIndex = PCPS->SeatIndex;
		PlayerBoard->OwnerPlayerState = PCPS;
		PlayerBoard->SetOwner(PCPS);
		PlayerBoard->PlayerBoardDelegate();
		
		UE_LOG(LogTemp, Warning, TEXT("PlayerBoardIndex : %d, PCPSSeatIndex : %d, OwnerPlayerState : %p "), PlayerBoard->PlayerIndex, PCPS->SeatIndex, PlayerBoard->OwnerPlayerState)
	}
}

APCPlayerBoard* APCCombatGameMode::FindPlayerBoardBySeat(int32 SeatIndex) const
{
	if (SeatIndex < 0)
		return nullptr;

	if (APCPlayerBoard* const* Found = SeatToPlayerBoard.Find(SeatIndex))
		return *Found;

	return nullptr;
}


APCCombatManager* APCCombatGameMode::GetCombatManager()
{
	if (CombatManager)
		return CombatManager;
	for (TActorIterator<APCCombatManager> It(GetWorld()); It; ++It)
	{
		CombatManager = *It;
		return CombatManager;
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


bool APCCombatGameMode::IsRoundSystemReady(FString& WhyNot) const
{
	APCCombatGameState* GS = GetCombatGameState();
	if (!IsValid(GS))
		{ WhyNot=TEXT("GameState null"); return false; }

	int32 NumPlayers=0;
	for (APlayerState* PSB : GS->PlayerArray)
	{
		const APCPlayerState* PS = Cast<APCPlayerState>(PSB);
		if (!PS) continue;
		++NumPlayers;

		if (PS->SeatIndex < 0)
			{ WhyNot = FString::Printf(TEXT("SeatIndex<0 PID=%d"), PS->GetPlayerId()); return false; }

		APCCombatBoard* Board = GS->GetBoardBySeat(PS->SeatIndex);
		if (!IsValid(Board))
			{ WhyNot = FString::Printf(TEXT("No Board for Seat %d"), PS->SeatIndex); return false; }
		if (!IsValid(Board->TileManager))
			{ WhyNot = FString::Printf(TEXT("No TileManager for Seat %d"), PS->SeatIndex); return false; }

		APCPlayerBoard* PlayerBoard = FindPlayerBoardBySeat(PS->SeatIndex);
		if (!IsValid(PlayerBoard))
		{
			WhyNot = FString::Printf(TEXT("No PlayerBoard for Seat %d"), PS->SeatIndex); return false;
		}

	}
	if (NumPlayers==0) { WhyNot=TEXT("No players"); return false; }

	if (!const_cast<APCCombatGameMode*>(this)->GetCombatManager())
	{ WhyNot=TEXT("CombatManager null"); return false; }

	if (!StageData) { WhyNot=TEXT("StageData null"); return false; }
	
	if (!IsValid(GS->GetShopManager()))
	{
		WhyNot = TEXT("Shop Manager null");
		return false;
	}
	
	
	return true;
}

void APCCombatGameMode::StartWhenReady()
{
	FString Why;
	if (IsRoundSystemReady(Why))
	{
		GetWorldTimerManager().ClearTimer(ThWaitReady);
		InitializeHomeBoardsForPlayers();
		BindPlayerBoardsToPlayerStates();
		BindPlayerAttribute();
		StartFromBeginning();
		return;
	}
	UE_LOG(LogTemp, Error, TEXT("StartWhenReady wait : %s"), *Why);
}

void APCCombatGameMode::AssignSeatDeterministicOnce()
{
	if (bSeatsFinalized) return;

	AGameStateBase* GS = GameState;
	if (!GS) return;

	TArray<APCPlayerState*> Players;
	for (APlayerState* PSB : GS->PlayerArray)
		if (auto* P = Cast<APCPlayerState>(PSB))
		{
			Players.Add(P);
			UE_LOG(LogTemp, Warning, TEXT("[Server Seat] %s PID=%d Seat=%d"),
					*P->GetPlayerName(), P->GetPlayerId(), P->SeatIndex);
		}
	

	// 1) 이미 배정된 좌석 중복 제거(중복이면 -1로 떨어뜨림), 사용 좌석 집계
	TSet<int32> Used;
	for (auto* P : Players)
	{
		if (P->SeatIndex >= 0)
		{
			if (Used.Contains(P->SeatIndex)) { P->SeatIndex = -1; }
			else { Used.Add(P->SeatIndex); }
		}
	}

	// 2) 결정적 순서(예: PlayerId)로 정렬 후 미배정 채우기
	Players.Sort([](const APCPlayerState& A, const APCPlayerState& B){
		return A.GetPlayerId() < B.GetPlayerId();
	});

	//const int32 MaxSeats = FMath::Max(1, GetTotalSeatSlots()); // 보드/링 개수 기반
	const int32 MaxSeats = 8;
	int32 next = 0;
	auto NextFree = [&](){
		while (Used.Contains(next)) ++next;
		return next % MaxSeats;
	};

	for (auto* P : Players)
	{
		if (P->SeatIndex < 0)
		{
			P->SeatIndex = NextFree();
			Used.Add(P->SeatIndex);
			P->ForceNetUpdate();
			UE_LOG(LogTemp, Warning, TEXT("Assigned SeatIndex=%d to PID=%d"), P->SeatIndex, P->GetPlayerId());
		}
	}

	// 좌석→보드 맵 재구축
	BuildHelperActor();
	bSeatsFinalized = true;
}

void APCCombatGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);
	const FString S = UGameplayStatics::ParseOption(Options, TEXT("ExpPlayers"));
	ExpectedPlayers = S.IsEmpty() ? 0 : FCString::Atoi(*S);
}

void APCCombatGameMode::OnOnePlayerArrived()
{
	if (ExpectedPlayers <= 0 && GameState)
		ExpectedPlayers = GameState->PlayerArray.Num(); // 보험

	++ArrivedPlayers;

	if (!bTriggeredAfterTravel && ExpectedPlayers > 0 && ArrivedPlayers >= ExpectedPlayers)
	{
		bTriggeredAfterTravel = true;
		// Pawn/PS/보드 참조 안정화용 한 틱 딜레이
		GetWorldTimerManager().SetTimerForNextTick([this]()
		{
			TryPlacePlayersAfterTravel();
		});
	}
}


// Carousel Helper
void APCCombatGameMode::BuildCarouselWavesByHP(TArray<TArray<int32>>& OutWaves)
{
	OutWaves.Reset();
	
	struct FSeatHp { int32 Seat; float Hp; int32 StableKey;};
	TArray<FSeatHp> Seats;

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (auto* PCPlayerController = Cast<APCCombatPlayerController>(*It))
		{
			if (auto* PCPlayerState = PCPlayerController->GetPlayerState<APCPlayerState>())
			{
				float Hp = 0.f;
				if (auto* PCASC = PCPlayerState->GetAbilitySystemComponent())
				{
					Hp = PCASC->GetNumericAttribute(UPCPlayerAttributeSet::GetPlayerHPAttribute());
					if (Hp <= 0.f)
					{
						continue;
					}
				}

				const int32 Stable = PCPlayerState->GetPlayerId();
				Seats.Add({PCPlayerState->SeatIndex, Hp, Stable});
			}
		}
	}

	Seats.Sort([](const FSeatHp& A, const FSeatHp& B)
	{
		if (!FMath::IsNearlyEqual(A.Hp, B.Hp))
		{
			return A.Hp < B.Hp;
		}
		return A.StableKey < B.StableKey;
	});

	for (int32 i = 0; i < Seats.Num(); i += 2)
	{
		TArray<int32> Wave;
		Wave.Add(Seats[i].Seat);
		if (i + 1 < Seats.Num())
		{
			Wave.Add(Seats[i + 1].Seat);
		}
		OutWaves.Add(Wave);
	}
}

void APCCombatGameMode::StartCarouselWaves()
{
	CurrentWaveIdx = -1;

	OpenCarouselWave(0);

	for (int32 w = 1; w < CarouselWaves.Num(); ++w)
	{
		const bool bLast = (w == CarouselWaves.Num() - 1);
		const float PrevTicks = (w - 1) * 5.f;
		const float Delay = bLast ? (PrevTicks + 5.f) : PrevTicks + 5.f;

		FTimerHandle Th;
		GetWorldTimerManager().SetTimer(Th, FTimerDelegate::CreateWeakLambda(this,[this, w]()
		{
			OpenCarouselWave(w);
		}),
		Delay, false
		);
	}

	const int32 WaveCnt = CarouselWaves.Num();
	const float EndDelay = (WaveCnt > 1) ? (WaveCnt - 1) * 5.f +8.f : 8.f;
	FTimerHandle ThEnd;
	GetWorldTimerManager().SetTimer(ThEnd, FTimerDelegate::CreateWeakLambda(this,[this]()
	{
		FinishCarouselRound();
	}),
	EndDelay, false
	);
}

void APCCombatGameMode::OpenCarouselWave(int32 WaveIdx)
{
	if (!CarouselRing || !CarouselWaves.IsValidIndex(WaveIdx)) return;

	CurrentWaveIdx = WaveIdx;

	for (int32 Seat : CarouselWaves[WaveIdx])
	{
		CarouselRing->Multicast_SetGateOpen(Seat, true);
	}

	const bool bLast = (WaveIdx == CarouselWaves.Num() - 1);
	StartSubWaveTimerUI(bLast ? 8.f : 5.f);
}

void APCCombatGameMode::StartSubWaveTimerUI(float DurationSeconds)
{
	if (auto* GS = GetCombatGameState())
	{
		FStageRuntimeState S;
		S.Stage            = EPCStageType::Carousel; // 그대로
		S.Duration         = DurationSeconds;
		S.ServerStartTime  = NowServer();
		S.ServerEndTime    = S.ServerStartTime + DurationSeconds;
		GS->SetStageRunTime(S);
	}
}

void APCCombatGameMode::FinishCarouselRound()
{
	EndCurrentStep();
}



