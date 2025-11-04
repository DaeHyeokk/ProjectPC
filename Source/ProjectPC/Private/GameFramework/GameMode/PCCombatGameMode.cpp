// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFramework/GameMode/PCCombatGameMode.h"

#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"

#include "BaseGameplayTags.h"
#include "AbilitySystem/Player/AttributeSet/PCPlayerAttributeSet.h"
#include "Character/Player/PCPlayerCharacter.h"
#include "Controller/Player/PCCombatPlayerController.h"
#include "GameFramework/GameState/PCCombatGameState.h"
#include "GameFramework/HelpActor/PCCarouselRing.h"
#include "GameFramework/HelpActor/PCCombatBoard.h"
#include "GameFramework/HelpActor/PCCombatManager.h"
#include "GameFramework/HelpActor/PCPlayerBoard.h"
#include "GameFramework/HelpActor/Component/PCTileManager.h"
#include "GameFramework/PlayerState/PCPlayerState.h"
#include "GameFramework/WorldSubsystem/PCUnitGERegistrySubsystem.h"
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
	EnterLoadingPhase();
	
			
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
}

void APCCombatGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
		
	if (auto* PC = Cast<APCCombatPlayerController>(NewPlayer))
	{
		PC->Client_RequestIdentity();
	}
	
	//OnOnePlayerArrived();
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
	case EPCStageType::PvPResult : Step_PvPResult(); break;
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

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (auto* PCPlayerController = Cast<APCCombatPlayerController>(*It))
		{
			PCPlayerController->Client_ShowPlayerMainWidget();
		}
	}
	
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
			if (APCPlayerState* PCPlayerState = PCPlayerController->GetPlayerState<APCPlayerState>())
			{
				if (NotReward)
				{
					PCPlayerController->Server_ShopRefresh(0);
				}
				else
				{
					PCPlayerState->ApplyRoundReward();
				}
			}			
		}
	}
}

void APCCombatGameMode::Step_Travel()
{
	const int32 Stage = FlatStageIdx.IsValidIndex(Cursor) ? FlatStageIdx[Cursor] : 0;
	const FRoundStep* Next = PeekNextStep();
	if (!Next) return;

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (APCCombatPlayerController* PCCombatPlayerController = Cast<APCCombatPlayerController>(*It))
		{
			PCCombatPlayerController->Client_RequestPlayerReturn();
		}
	}

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
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (APCCombatPlayerController* PCCombatPlayerController = Cast<APCCombatPlayerController>(*It))
		{
			PCCombatPlayerController->Client_RequestPlayerReturn();
		}
	}
	
	const FRoundStep* Prev = PeekPrevStep();
	if (!Prev)
	{
		MovePlayersToBoardsAndCameraSet();
		return;
	}
	if (Prev->StageType == EPCStageType::PvPResult)
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

	if (!CombatManager) return;
	CombatManager->CheckVictory();
}

void APCCombatGameMode::Step_PvPResult()
{
	if (!CombatManager) return;
	CombatManager->HandleBattleFinished();
}

void APCCombatGameMode::Step_PvE()
{
	if (APCCombatGameState* PCGameState = GetCombatGameState())
	{
		PCGameState->SetGameStateTag(GameStateTags::Game_State_Combat_Active);
	}

	if (!CombatManager) return;
	CombatManager->CheckVictory();
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
	APCCombatGameState* PCGameState = GetCombatGameState();
	if (!PCGameState) return;

	PCGameState->SetGameStateTag(GameStateTags::Game_State_Carousel);
	
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
					// PCPlayerState->UnitSpawn(SpawnTag[SpawnIndex]);
					PCPlayerState->UnitSpawn(UnitGameplayTags::Unit_Type_Hero_IggyScorch);
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
		
		PCPlayerState->ChangeState(PlayerGameplayTags::Player_State_Carousel);

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

		PlayerState->ChangeState(PlayerGameplayTags::Player_State_Normal);

		const int32 BoardIdx = ResolveBoardIndex(PlayerState);
		APCCombatBoard* Board = CombatBoard.IsValidIndex(BoardIdx) ? CombatBoard[BoardIdx] : nullptr;
		if (!Board) continue;

		if (APawn* Pawn = PlayerState->GetPawn())
		{
			const FTransform Seat = Board->GetPlayerSeatTransform();
			Pawn->TeleportTo(Seat.GetLocation(), Pawn->GetActorRotation(), false, true);
		}
		
		PlayerController->ClientFocusBoardBySeatIndex(BoardIdx, 0);
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


void APCCombatGameMode::PollPreStartBarrier()
{
	if (APCCombatGameState* GS = GetGameState<APCCombatGameState>())
	{
		int32 Ready = 0;
		int32 Total = 0;
		if (GS->AreAllLoadingUIClosed(Ready, Total) && Total>0 && Ready==Total)
		{
			FinishPreStartAndSchedule();
		}
	}
}

void APCCombatGameMode::FinishPreStartAndSchedule()
{
	GetWorldTimerManager().ClearTimer(ThPreStartBarrier);
	GetWorldTimerManager().ClearTimer(ThArmTimeout);

	APCCombatGameState* GS = GetGameState<APCCombatGameState>();
	if (!GS) return;

	GS->SetLoadingState(false, 1.f, TEXT("Ready"));

	const double Now = GS->GetServerWorldTimeSeconds();
	const double TStart = GS->StepArmTimeWS;
	const float Delay = FMath::Max(0.f, TStart-Now);

	GetWorldTimerManager().SetTimer(ThStartAt, this, &ThisClass::StartFromBeginning, Delay, false);

	FStageRuntimeState S;
	S.Stage = EPCStageType::Start;
	S.ServerStartTime = TStart;
	S.ServerEndTime = TStart + (StageData ? StageData->GetDefaultDuration(EPCStageType::Start) : 1.f);
	GS->SetStageRunTime(S);
}

void APCCombatGameMode::ForceShortenCurrentStep(float NewRemainingSeconds)
{
	if (!GetCombatGameState()) return;
	NewRemainingSeconds = FMath::Max(0.1f, NewRemainingSeconds);

	GetWorldTimerManager().ClearTimer(RoundTimer);
	GetWorldTimerManager().SetTimer(RoundTimer, this, &APCCombatGameMode::EndCurrentStep, NewRemainingSeconds, false);

	FStageRuntimeState S = GetCombatGameState()->GetStageRunTime();
	const double Now = NowServer();
	const float Elapsed = FMath::Max(0.f, Now - S.ServerStartTime);
	S.Duration = Elapsed + NewRemainingSeconds;
	S.ServerEndTime = Now + NewRemainingSeconds;
	GetCombatGameState()->SetStageRunTime(S);
}

void APCCombatGameMode::EnterLoadingPhase()
{
	if (APCCombatGameState* GS = GetCombatGameState())
	{
		GS->SetLoadingState(true, 0.f, TEXT("Waiting for Players.."));
	}

	GetWorldTimerManager().SetTimer(ThLoadingPoll, this, &ThisClass::PollLoading, 0.25f, true, 0.0f);
}

void APCCombatGameMode::PollLoading()
{
	APCCombatGameState* GS = GetCombatGameState();
	if (!GS) return;

	// 1) 인원 접속 확인
	const int32 Expected = (ExpectedPlayers > 0) ? ExpectedPlayers : (GS ? GS->PlayerArray.Num() : 0);
	const int32 Connected = GS ? GS->PlayerArray.Num() : 0;
	const float P1 = (Expected > 0) ? FMath::Clamp(Connected / Expected, 0.f, 1.0f) : 0.f;

	// 2) ID 입력 확인
	int32 Ready = 0;
	int32 Total = 0;
	AreAllPlayersIdentified(Ready, Total);
	const float P2 = (Total > 0) ? Ready / Total : 0.f;

	// ★★★ 좌석/보드 준비는 'ID가 모두 준비된 뒤' 딱 1회만 수행
	if (P2 >= 1.f)
	{
		AssignSeatDeterministicOnce();   // SeatIndex 확정
		BuildHelperActor();              // Seat->Board 맵 재구축
		CollectPlayerBoards();           // Seat->PlayerBoard 캐시
	}

	// 3) 보드 / 타일 / 플레이어보드 확인
	bool bBoardsOK = true;
	if (GS)
	{
		for (APlayerState* PSB : GS->PlayerArray)
		{
			const APCPlayerState* PCPS = Cast<APCPlayerState>(PSB);
			if (!PCPS) continue;
			if (PCPS->SeatIndex < 0)
			{
				bBoardsOK = false;
				break;
			}
			APCCombatBoard* Board = GS->GetBoardBySeat(PCPS->SeatIndex);
			if (!IsValid(Board) || !IsValid(Board->TileManager))
			{
				bBoardsOK = false;
				break;
			}
			if (!FindPlayerBoardBySeat(PCPS->SeatIndex))
			{
				bBoardsOK = false;
				break;
			}
		}
	}
	
	const float P3 = bBoardsOK ? 1.f : 0.f;

	// 4) 서브시스템 / 스테이지 / 샵 매니저
	bool bSystems = true;
	if (!GetCombatManager())
	{
		bSystems = false;
	}
	if (!StageData)
	{
		bSystems = false;
	}
	if (!IsValid(GS->GetShopManager()))
	{
		bSystems = false;
	}

	const float P4 = bSystems ? 1.f : 0.f;

	// UI 바인딩전 LeaderBoard 생성
	if (!bAttributesBound && P1 >= 1.f && P2 >= 1.f)
	{
		BindPlayerAttribute();
		bAttributesBound = true;
		GS->SetLoadingState(true, 0.80f, TEXT("Seeding LeaderBoard..."));
		return;
	}

	// 5) 클라 UI 동기화 확인
	const float P5 = GS->ClientBootstrapRatio();

	const float Progress = W_Connected * P1 + W_Identified * P2 + W_Board * P3 +
		W_Systems * P4 + W_ClientUI * P5;

	FString Detail;
	if (Connected < Expected)       Detail = FString::Printf(TEXT("Players %d/%d connected"), Connected, Expected);
	else if (Ready < Total)         Detail = FString::Printf(TEXT("Identities %d/%d ready"), Ready, Total);
	else if (!bBoardsOK)            Detail = TEXT("Boards/TileManagers/PlayerBoards not ready");
	else if (!bSystems)             Detail = TEXT("Systems/Stage/Shop not ready");
	else if (P5 < 1.f)              Detail = TEXT("Clients binding UI…");
	else                            Detail = TEXT("Ready");

	GS->SetLoadingState(true, Progress, Detail);

	if (Progress >= 1.f)
	{
		ExitLoadingPhaseAndStart();
	}
	
}

void APCCombatGameMode::ExitLoadingPhaseAndStart()
{
	GetWorldTimerManager().ClearTimer(ThLoadingPoll);
	
	InitializeHomeBoardsForPlayers();
	BindPlayerBoardsToPlayerStates();

	// === OverheadWidget 보정 ===
	for (TActorIterator<APCPlayerCharacter> It(GetWorld()); It; ++It)
	{
		It->Multicast_SetOverHeadWidget();
	}

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (auto* PCPlayerController = Cast<APCCombatPlayerController>(*It))
		{
			PCPlayerController->TryInitWidgetWithGameState();
		}
	}

	APCCombatGameState* GS = GetCombatGameState();
	if (!GS) return;
	
	// 1) Tstart: 서버 월드 시간 기준으로 약간 미래(1.5초 권장)
	const double Now    = GS->GetServerWorldTimeSeconds(); // 또는 GetWorld()->GetTimeSeconds()
	const double Tstart = Now + 1.50;

	// 2) 시작 예고 (여전히 bLoading=true 유지)
	GS->SetLoadingState(true, 0.99f, TEXT("Starting…"));
	GS->ArmStepStart(Tstart);

	// 3) Barrier: 모든 클라가 UI 닫았는지 0.05s 간격으로 확인
	GetWorldTimerManager().SetTimer(ThPreStartBarrier, this, &ThisClass::PollPreStartBarrier, 0.05f, true, 0.0f);

	// 4) 안전 타임아웃: Tstart 직전까지 ACK가 다 안 오면 강행
	const float ArmTimeout = FMath::Max(0.1f, float(Tstart - Now) - 0.2f);
	GetWorldTimerManager().SetTimer(ThArmTimeout, [this]()
	{
		FinishPreStartAndSchedule();
	}, ArmTimeout, false);
}

bool APCCombatGameMode::IsRoundSystemReady(FString& WhyNot) 
{
	 APCCombatGameState* GS = GetCombatGameState();
    if (!IsValid(GS))
        { WhyNot=TEXT("GameState null"); return false; }

    // 3-1) 예상 인원 계산
    const int32 Expected = (ExpectedPlayers > 0) ? ExpectedPlayers
                      : (GameState ? GameState->PlayerArray.Num() : 0);
    if (Expected <= 0)
        { WhyNot = TEXT("ExpectedPlayers <= 0"); return false; }

    // 3-2) 플레이어 수 도달 여부(연결 전)
    const int32 CurrentNum = GameState->PlayerArray.Num();
    if (CurrentNum < Expected)
    {
        WhyNot = FString::Printf(TEXT("Waiting players: %d/%d connected"), CurrentNum, Expected);
        return false;
    }

    // 3-3) **LocalUserId 전원 준비됐는지** (핵심)
    int32 Ready = 0, Total = 0;
    AreAllPlayersIdentified(Ready, Total);
    if (Ready < Expected)
    {
        WhyNot = FString::Printf(TEXT("Waiting identities: %d/%d ready"), Ready, Expected);
        return false;
    }

    // 3-4) 이후 기존 검사들 유지
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
            { WhyNot = FString::Printf(TEXT("No PlayerBoard for Seat %d"), PS->SeatIndex); return false; }
    }
    if (NumPlayers==0) { WhyNot=TEXT("No players"); return false; }

    if (!GetCombatManager())
        { WhyNot=TEXT("CombatManager null"); return false; }

    if (!StageData)
        { WhyNot=TEXT("StageData null"); return false; }
    
    if (!IsValid(GS->GetShopManager()))
        { WhyNot = TEXT("Shop Manager null"); return false; }
    
    return true;
}

bool APCCombatGameMode::AreAllPlayersIdentified(int32& OutReady, int32& OutTotal) const
{
	OutReady = 0;
	OutTotal = 0;

	const AGameStateBase* GS = GameState;
	if (!GS) return false;

	for (APlayerState* PSB : GS->PlayerArray)
	{
		if (const APCPlayerState* PS = Cast<APCPlayerState>(PSB))
		{
			++OutTotal;
			const bool bHasId = !PS->LocalUserId.IsEmpty();
			if (bHasId)
			{
				++OutReady;
			}
			else
			{
				UE_LOG(LogTemp, Verbose, TEXT("[ReadyCheck] LocalUserId empty: PID=%d Seat=%d PS=%s"),
					PS->GetPlayerId(), PS->SeatIndex, *PS->GetName());
			}
		}
	}
	return (OutTotal > 0) && (OutReady == OutTotal);
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
	AGameStateBase* GS = GameState;
	if (!GS) return;

	TArray<APCPlayerState*> Players;
	for (APlayerState* PSB : GS->PlayerArray)
		if (auto* P = Cast<APCPlayerState>(PSB))
		{
			Players.Add(P);
		}
	

	// 1) 이미 배정된 좌석 중복 제거(중복이면 -1로 떨어뜨림), 사용 좌석 집계
	TSet<int32> Used;
	for (auto* P : Players)
	{
		if (P->SeatIndex >= 0)
		{
			if (Used.Contains(P->SeatIndex))
			{
				P->SeatIndex = -1;
			}
			else
			{
				Used.Add(P->SeatIndex);
			}
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
			P->SetCurrentSeatIndex(P->SeatIndex);
			
			P->ForceNetUpdate();
			UE_LOG(LogTemp, Warning, TEXT("Assigned SeatIndex=%d to PID=%d"), P->SeatIndex, P->GetPlayerId());
		}
	}

	
	// 좌석→보드 맵 재구축
	BuildHelperActor();
}

void APCCombatGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);
	const FString S = UGameplayStatics::ParseOption(Options, TEXT("ExpPlayers"));
	ExpectedPlayers = S.IsEmpty() ? 0 : FCString::Atoi(*S);
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
		FStageRuntimeState S = GS->GetStageRunTime();
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



