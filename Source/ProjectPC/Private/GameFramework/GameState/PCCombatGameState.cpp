// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFramework/GameState/PCCombatGameState.h"

#include "EngineUtils.h"
#include "Net/UnrealNetwork.h"

#include "AbilitySystem/Player/AttributeSet/PCPlayerAttributeSet.h"
#include "GameFramework/HelpActor/PCCombatBoard.h"
#include "GameFramework/HelpActor/PCCombatManager.h"
#include "GameFramework/PlayerState/PCPlayerState.h"
#include "GameFramework/WorldSubsystem/PCItemManagerSubsystem.h"
#include "GameFramework/WorldSubsystem/PCItemSpawnSubsystem.h"
#include "GameFramework/WorldSubsystem/PCProjectilePoolSubsystem.h"
#include "GameFramework/WorldSubsystem/PCUnitCombatTextSpawnSubsystem.h"
#include "GameFramework/WorldSubsystem/PCUnitSpawnSubsystem.h"
#include "Item/PCItemCapsule.h"
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

	if (auto* DamageTextSpawnSubsystem = GetWorld()->GetSubsystem<UPCUnitCombatTextSpawnSubsystem>())
	{
		DamageTextSpawnSubsystem->InitCombatTextSpawnSubsystem(CombatTextClass);
	}
	
	if (auto* ProjectilePoolSubsystem = GetWorld()->GetSubsystem<UPCProjectilePoolSubsystem>())
	{
		if (ProjectilePoolData)
		{
			ProjectilePoolSubsystem->InitializeProjectilePoolData(ProjectilePoolData->ProjectilePoolData);
		}
	}

	if (auto* ItemManagerSubsystem = GetWorld()->GetSubsystem<UPCItemManagerSubsystem>())
	{
		if (ItemDataTable && ItemCombineDataTable)
		{
			ItemManagerSubsystem->InitializeItemManager(ItemDataTable, ItemCombineDataTable);
		}
	}

	if (auto* ItemSpawnSubsystem = GetWorld()->GetSubsystem<UPCItemSpawnSubsystem>())
	{
		if (ItemCapsuleClass)
		{
			ItemSpawnSubsystem->InitializeItemCapsuleClass(ItemCapsuleClass);
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

void APCCombatGameState::ArmStepStart(double InServerWorldStartTime)
{
	if (!HasAuthority()) return;
	bStepArmed = true;
	StepArmTimeWS = InServerWorldStartTime;
	
}

void APCCombatGameState::Server_ReportUILoadingClosed_Implementation(const FString& LocalUserId)
{
	if (!HasAuthority() || LocalUserId.IsEmpty()) return;
	FUILoadingFlags& Flags = UILoadingById.FindOrAdd(LocalUserId);
	Flags.bClosed = true;
	Flags.LastUpdate = GetServerWorldTimeSeconds();
}

bool APCCombatGameState::AreAllLoadingUIClosed(int32& OutReady, int32& OutTotal) const
{
	OutReady = 0;
	OutTotal = 0;
	for (APlayerState* PSB : PlayerArray)
	{
		if (const APCPlayerState* PCPS = Cast<APCPlayerState>(PSB))
		{
			++OutTotal;
			const FUILoadingFlags* Flags = UILoadingById.Find(PCPS->LocalUserId);
			if (Flags && Flags->bClosed)
			{
				++OutReady;
			}
		}
	}
	return (OutTotal > 0) && (OutReady == OutTotal);
}

void APCCombatGameState::SetLoadingState(bool bInLoading, float InProgress, const FString& InDetail)
{
	if (!HasAuthority()) return;
	bLoading = bInLoading;
	LoadingProgress = FMath::Clamp(InProgress, 0.f, 1.0f);
	LoadingDetail = InDetail;
	OnRep_Loading();
	
}

void APCCombatGameState::OnRep_Loading()
{
	OnLoadingChanged.Broadcast();
}

void APCCombatGameState::Server_UpdateBootstrap(const FString& LocalUserId, uint8 Mask)
{
	if (!HasAuthority() || LocalUserId.IsEmpty()) return;

	FBootstrapFlags& Flag = BootstrapById.FindOrAdd(LocalUserId);
	Flag.Mask = Mask;
	Flag.LastUpdate = GetServerWorldTimeSeconds();
}

bool APCCombatGameState::AreAllClientsBootstrapped(int32& OutReady, int32& OutTotal) const
{
	OutReady = 0;
	OutTotal = 0;

	for (APlayerState* PlayerState : PlayerArray)
	{
		const APCPlayerState* PCPlayerState = Cast<APCPlayerState>(PlayerState);
		if (!PCPlayerState) continue;

		++OutTotal;

		const FBootstrapFlags* Flag = BootstrapById.Find(PCPlayerState->LocalUserId);
		if (Flag && Flag->All())
		{
			++OutReady;
		}
	}
	return (OutTotal > 0) && (OutReady == OutTotal);
}

float APCCombatGameState::ClientBootstrapRatio() const
{
	int32 Ready = 0;
	int32 Total = 0;
	AreAllClientsBootstrapped(Ready, Total);
	return (Total > 0) ? Ready / Total : 0.0f;
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

void APCCombatGameState::SetRoundsPerStage(const TArray<int32>& InCounts)
{
	if (HasAuthority())
	{
		RoundsPerStage = InCounts;
		OnRep_RoundsLayout();
		ForceNetUpdate();
	}
}



void APCCombatGameState::SetRoundMajorsFlat(const TArray<FGameplayTag>& InFlatMajors)
{
	RoundMajorFlat = InFlatMajors;
	OnRep_RoundsLayout();
	ForceNetUpdate();
}

int32 APCCombatGameState::GetNumRoundsInStage(int32 StageIdx) const
{
	return RoundsPerStage.IsValidIndex(StageIdx) ? RoundsPerStage[StageIdx] : 0;
}

int32 APCCombatGameState::StagesStartFlatIndex(int32 StageIdx) const
{
	if (RoundsPerStage.Num() <= 0)
		return 0;

	int32 Acc = 0;
	for (int32 Stage = 0; Stage<StageIdx && Stage < RoundsPerStage.Num(); ++Stage)
	{
		Acc += FMath::Max(0, RoundsPerStage[Stage]);
	}

	return Acc;
}

void APCCombatGameState::OnRep_RoundResult()
{
	OnStageRuntimeChanged.Broadcast();
}

void APCCombatGameState::ApplyRoundResultForSeat(int32 SeatIdx, int32 StageIdx, int32 RoundIdx, ERoundResult Result)
{
	const int32 TotalRounds = TotalRoundsFloat();
	if (TotalRounds <= 0) return;

	const int32 FlatRound = StagesStartFlatIndex(StageIdx) + RoundIdx;
	if (SeatIdx < 0 || FlatRound < 0) return;
	const int32 Flat = SeatIdx * TotalRounds + FlatRound;

	if (SeatRoundResult.Num() <= Flat)
	{
		SeatRoundResult.SetNum(Flat + 1, false);
	}

	SeatRoundResult[Flat] = Result;

	OnRep_RoundResult();
	ForceNetUpdate();
}

int32 APCCombatGameState::GetMySeatIndex() const
{
	if (const APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr)
	{
		if (const APCPlayerState* PS = PC->GetPlayerState<APCPlayerState>())
		{
			return PS->SeatIndex;
		}
	}
	return -1;
}


ERoundResult APCCombatGameState::GetRoundResultForSeat(int32 SeatIdx, int32 StageIdx, int32 RoundIdx) const
{
	const int32 TotalRounds = TotalRoundsFloat();
	const int32 FlatRound = StagesStartFlatIndex(StageIdx) + RoundIdx;
	const int32 Flat = SeatIdx * TotalRounds + FlatRound;

	if (SeatRoundResult.IsValidIndex(Flat))
	{
		return SeatRoundResult[Flat];
	}

	return ERoundResult::None;
}

bool APCCombatGameState::WasRoundVictory(int32 StageIdx, int32 RoundIdx) const
{
	return GetRoundResultForSeat(GetMySeatIndex(), StageIdx, RoundIdx) == ERoundResult::Victory;
}

bool APCCombatGameState::WasRoundDefeat(int32 StageIdx, int32 RoundIdx) const
{
	return GetRoundResultForSeat(GetMySeatIndex(), StageIdx, RoundIdx) == ERoundResult::Defeat;
}

bool APCCombatGameState::WasRoundDraw(int32 StageIdx, int32 RoundIdx) const
{
	return GetRoundResultForSeat(GetMySeatIndex(),StageIdx, RoundIdx) == ERoundResult::Draw;
}

int32 APCCombatGameState::TotalRoundsFloat() const
{
	int32 Sum = 0;
	for (int32 V : RoundsPerStage)
	{
		Sum += FMath::Max(0, V);
	}
	return Sum;
}

FGameplayTag APCCombatGameState::GetMajorStageForRound(int32 StageIdx, int32 RoundIdx) const
{
	const int32 flat = StagesStartFlatIndex(StageIdx) + RoundIdx;
	return RoundMajorFlat.IsValidIndex(flat) ? RoundMajorFlat[flat] : FGameplayTag();
}

FGameplayTag APCCombatGameState::GetPvETagForRound(int32 StageIdx, int32 RoundIdx) const
{
	const int32 flat = StagesStartFlatIndex(StageIdx) + RoundIdx;
	return RoundPvETagFlat.IsValidIndex(flat) ? RoundPvETagFlat[flat] : FGameplayTag();
}

void APCCombatGameState::OnRep_RoundsLayout()
{
	const int32 Total = TotalRoundsFloat();
	if (RoundMajorFlat.Num() < Total)
	{
		RoundMajorFlat.SetNum(Total);
	}

	if (RoundPvETagFlat.Num() < Total)
	{
		RoundPvETagFlat.SetNum(Total);
	}

	OnRoundsLayoutChanged.Broadcast();
}

void APCCombatGameState::OnRep_StageRunTime()
{
	OnStageRuntimeChanged.Broadcast();
}


void APCCombatGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// 로딩 관련
	DOREPLIFETIME(APCCombatGameState, bLoading);
	DOREPLIFETIME(APCCombatGameState, LoadingProgress);
	DOREPLIFETIME(APCCombatGameState, LoadingDetail);

	DOREPLIFETIME_CONDITION_NOTIFY(APCCombatGameState, GameStateTag, COND_None, REPNOTIFY_OnChanged);
	DOREPLIFETIME(APCCombatGameState, StageRuntimeState);
	DOREPLIFETIME(APCCombatGameState, SeatToBoard);
	DOREPLIFETIME(APCCombatGameState, bBoardMappingComplete);
	DOREPLIFETIME(APCCombatGameState, Leaderboard);
	DOREPLIFETIME(APCCombatGameState, FindPlayerStates);

	DOREPLIFETIME(APCCombatGameState, RoundsPerStage);
	DOREPLIFETIME(APCCombatGameState, RoundMajorFlat);
	DOREPLIFETIME(APCCombatGameState, RoundPvETagFlat);
	DOREPLIFETIME(APCCombatGameState, SeatRoundResult);

	
	
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

void APCCombatGameState::BindAllPlayerHP()
{
	if (!HasAuthority()) return;

	AliveCount = 0;

	for (APlayerState* PS : PlayerArray)
	{
		if (APCPlayerState* PCPlayerState = Cast<APCPlayerState>(PS))
		{
			BindOnePlayerHpDelegate(PCPlayerState);
		}
	}

	RebuildAndReplicatedLeaderboard();
}

void APCCombatGameState::BindOnePlayerHpDelegate(APCPlayerState* PCPlayerState)
{
	if (!HasAuthority() || !IsValid(PCPlayerState)) return;
	
	UAbilitySystemComponent* ASC = ResolveASC(PCPlayerState);
	if (!ASC) return;
	
	// 중복방지
	if (HpDelegateHandles.Contains(ASC)) return;
	
	const FString& Id = PCPlayerState->LocalUserId;
	const float Now = GetServerWorldTimeSeconds();
	
	// 최초 관측 순서 기록
	if (!StableOrderCache.Contains(Id))
	{
		StableOrderCache.Add(Id,++StableOrderCounter);
	}
	
	// 초기 HP 캐시
	const float CurHP = ASC->GetNumericAttribute(UPCPlayerAttributeSet::GetPlayerHPAttribute());
	HpCache.FindOrAdd(Id) = CurHP;
	LastChangeTimeCache.FindOrAdd(Id) = Now;
	
	if (CurHP <= 0.f)
	{
		EliminatedSet.Add(Id);
	}
	else
	{
		++AliveCount;
	}
	
	// AttributeChangeDelegate
	FDelegateHandle Handle = ASC->GetGameplayAttributeValueChangeDelegate(UPCPlayerAttributeSet::GetPlayerHPAttribute())
	.AddLambda([this, PCPlayerState](const FOnAttributeChangeData& Data)
	{
		if (!this || !this->HasAuthority() || !IsValid(PCPlayerState)) return;
	
		const float NewHp = Data.NewValue;
		OnHpChanged_Server(PCPlayerState, NewHp);
	
		if (NewHp <= 0.f)
		{
			OnEliminated_Server(PCPlayerState);
		}
	});
	
	HpDelegateHandles.Add(ASC,Handle);	
}

int32 APCCombatGameState::AssignFinalRankOnDeathById(const FString& LocalUserId)
{
	if (!HasAuthority())
	{
		// 클라 : 이미 확정된게 있으면 반환, 아니면 0
		if (const int32* Existing = FinalRanks.Find(LocalUserId))
		{
			return *Existing;
		}
		return 0;
	}

	if (const int32* Existing = FinalRanks.Find(LocalUserId))
	{
		return *Existing;
	}

	if (!EliminatedSet.Contains(LocalUserId))
	{
		EliminatedSet.Add(LocalUserId);
		AliveCount = FMath::Max(0, AliveCount - 1);
	}

	const int32 Total = PlayerArray.Num();
	const int32 Already = FinalRanks.Num();
	const int32 ThisRank = FMath::Max(1, Total - Already);

	FinalRanks.Add(LocalUserId, ThisRank);

	RebuildAndReplicatedLeaderboard();

	if (AliveCount <= 1)
	{
		TryFinalizeLastSurvivor();
	}

	return ThisRank;
}

int32 APCCombatGameState::AssignFinalRankOnDeathByPS(APCPlayerState* PCPlayerState)
{
	if (!PCPlayerState) return 0;
	return AssignFinalRankOnDeathById(PCPlayerState->LocalUserId);
}

int32 APCCombatGameState::GetFinalRankFor(const FString& LocalUserId) const
{
	if (const int32* R = FinalRanks.Find(LocalUserId))
		return *R;
	return 0;
}

void APCCombatGameState::RemovePlayerState(APlayerState* PlayerState)
{
	if (APCPlayerState* PCS = Cast<APCPlayerState>(PlayerState))
	{
		if (UAbilitySystemComponent* ASC = ResolveASC(PCS))
		{
			if (FDelegateHandle* Handle = HpDelegateHandles.Find(ASC))
			{
				ASC->GetGameplayAttributeValueChangeDelegate(UPCPlayerAttributeSet::GetPlayerHPAttribute())
				.Remove(*Handle);
				HpDelegateHandles.Remove(ASC);
			}
		}

		const FString& ID = PCS->LocalUserId;

		if (!EliminatedSet.Contains(ID))
		{
			AliveCount = FMath::Max(0, AliveCount -1);
		}

		RebuildAndReplicatedLeaderboard();
	}
	Super::RemovePlayerState(PlayerState);
}

void APCCombatGameState::OnHpChanged_Server(APCPlayerState* PCPlayerState, float NewHp)
{
	const FString& Id = PCPlayerState->LocalUserId;
	HpCache.FindOrAdd(Id) = NewHp;
	LastChangeTimeCache.FindOrAdd(Id) = GetServerWorldTimeSeconds();
	RebuildAndReplicatedLeaderboard();
}

void APCCombatGameState::OnEliminated_Server(APCPlayerState* PCPlayerState)
{
	AssignFinalRankOnDeathByPS(PCPlayerState);
}

void APCCombatGameState::RebuildAndReplicatedLeaderboard()
{
	if (!HasAuthority())
		return;

	const int32 Total = PlayerArray.Num();
	if (Total <= 0)
	{
		Leaderboard.Reset();
		ForceNetUpdate();
		return;
	}

	TMap<FString, FPlayerStandingRow> RowById;
	RowById.Reserve(Total);

	for (APlayerState* PlayerState : PlayerArray)
	{
		if (APCPlayerState* PCS = Cast<APCPlayerState>(PlayerState))
		{
			const FString& Id = PCS->LocalUserId;

			UE_LOG(LogTemp, Warning, TEXT("[leaderboard] LocalUserId : %s"), *Id)

			FPlayerStandingRow Row;
			Row.LocalUserId = Id;
			Row.PlayerSeatIndex = PCS->SeatIndex;
			Row.Hp = HpCache.FindRef(Id);
			Row.bEliminated = EliminatedSet.Contains(Id);
			Row.FinalRank = FinalRanks.FindRef(Id);
			Row.StableOrder = StableOrderCache.FindRef(Id);
			Row.LastChangeTime = LastChangeTimeCache.FindRef(Id);
			Row.LiveRank = 0;
			
			if (auto ASC = PCS->GetAbilitySystemComponent())
			{
				FGameplayTagContainer PlayerTags;
				ASC->GetOwnedGameplayTags(PlayerTags);

				for (const FGameplayTag& Tag : PlayerTags)
				{
					if (Tag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("Player.Type"))))
					{
						Row.CharacterTag = Tag;
						break;
					}
				}
			}

			RowById.Add(Id, Row);
		}
	}

	// 결과 배열을 전체 인원 수로 고정

	TArray<FPlayerStandingRow> NewReaderBoard;
	NewReaderBoard.SetNum(Total);

	// 빈칸 목록
	TArray<int32> EmptySlots;
	EmptySlots.Reserve(Total);
	for (int32 i = 0; i < Total; ++i)
	{
		EmptySlots.Add(i);
	}

	for (auto& KVP : RowById)
	{
		FPlayerStandingRow& Row = KVP.Value;
		if (Row.FinalRank > 0)
		{
			const int32 SlotIdx = FMath::Clamp(Row.FinalRank -1, 0, Total -1);
			NewReaderBoard[SlotIdx] = Row;
			EmptySlots.Remove(SlotIdx);
		}
	}

	// 생존자만 추려서 HP 내림차순
	TArray<FPlayerStandingRow> AliveRows;
	AliveRows.Reserve(Total);
	for (auto& KVP : RowById)
	{
		const FPlayerStandingRow& Row = KVP.Value;
		if (Row.FinalRank == 0)
		{
			AliveRows.Add(Row);
		}
	}

	AliveRows.Sort([] (const FPlayerStandingRow& A, const FPlayerStandingRow& B)
	{
		if (!FMath::IsNearlyEqual(A.Hp, B.Hp))
			return A.Hp > B.Hp;
		if (!FMath::IsNearlyEqual(A.LastChangeTime, B.LastChangeTime))
			return A.LastChangeTime > B.LastChangeTime;

		return A.LocalUserId < B.LocalUserId;
	});

	int32 LiveRankCounter = 1;
	int32 EmptyIdxIter = 0;

	for (const FPlayerStandingRow& Alive : AliveRows)
	{
		if (!EmptySlots.IsValidIndex(EmptyIdxIter))
			break;

		const int32 SlotIdx = EmptySlots[EmptyIdxIter++];

		FPlayerStandingRow Placed = Alive;
		Placed.LiveRank = LiveRankCounter++;
		NewReaderBoard[SlotIdx] = Placed;
	}

	while (EmptyIdxIter < EmptySlots.Num())
	{
		const int32 SlotIdx = EmptySlots[EmptyIdxIter++];
		FPlayerStandingRow PlaceHolder;
		PlaceHolder.LocalUserId = TEXT("");
		NewReaderBoard[SlotIdx] = PlaceHolder;
	}

	Leaderboard = MoveTemp(NewReaderBoard);
	GetPlayerStatesOrdered();
	
	ForceNetUpdate();
	
}

void APCCombatGameState::TryFinalizeLastSurvivor()
{
	if (!HasAuthority()) return;

	int32 FinalRankNum = FinalRanks.Num();
	int32 PlayerNum = PlayerArray.Num();

	if (FinalRankNum >= PlayerNum)
		return;

	for (APlayerState* PlayerState : PlayerArray)
	{
		if (APCPlayerState* PCPlayerState = Cast<APCPlayerState>(PlayerState))
		{
			const FString& Id = PCPlayerState->LocalUserId;
			if (!EliminatedSet.Contains(Id))
			{
				FinalRanks.Add(Id,1);
				PCPlayerState->PlayerResult();
				break;
			}
		}
	}

	RebuildAndReplicatedLeaderboard();
}

APCPlayerState* APCCombatGameState::FindPlayerStateByUserId(const FString& LocalUserId) const
{
	if (LocalUserId.IsEmpty()) return nullptr;

	for (APlayerState* PlayerState : PlayerArray)
	{
		if (APCPlayerState* PCPlayerState = Cast<APCPlayerState>(PlayerState))
		{
			if (PCPlayerState->LocalUserId == LocalUserId)
			{
				return PCPlayerState;
			}
		}
	}
	return nullptr;
}

void APCCombatGameState::GetPlayerStatesOrdered() 
{
	FindPlayerStates.Reset();

	if (Leaderboard.Num() <= 0) return;

	FindPlayerStates.SetNum(Leaderboard.Num());

	for (int32 i = 0; i < Leaderboard.Num(); ++i)
	{
		const FString& Id = Leaderboard[i].LocalUserId;
		FindPlayerStates[i] = FindPlayerStateByUserId(Id);
	}
	
	
}

void APCCombatGameState::OnRep_Leaderboard()
{
	BroadCastLeaderboardMap();
	GetPlayerStatesOrdered();
	
	if (!bLeaderBoardReady)
	{
		bLeaderBoardReady = true;
		OnLeaderBoardReady.Broadcast();
	}

	FindPlayerState.Broadcast(FindPlayerStates);
	
}

UAbilitySystemComponent* APCCombatGameState::ResolveASC(APCPlayerState* PCPlayerState) const
{
	if (!PCPlayerState)
		return nullptr;

	return PCPlayerState->GetAbilitySystemComponent();
}

void APCCombatGameState::BroadCastLeaderboardMap()
{
	FLeaderBoardMap Map;
	Map.Reserve(Leaderboard.Num());
	for (const FPlayerStandingRow& Row : Leaderboard)
	{
		if (!Row.LocalUserId.IsEmpty())
		{
			Map.Add(Row.LocalUserId, Row);
		}
	}
	CachedLeaderboardMap = Map;
	OnLeaderboardMapUpdated.Broadcast(Map);
}

// 한 줄 포맷터 헬퍼
static FString FormatRowLine(int32 SlotIndex, const FPlayerStandingRow& Row)
{
	// SlotIndex: 0=1등 칸
	const int32 SlotRank = SlotIndex + 1;
	const FString Status  = Row.FinalRank > 0 ? TEXT("DEAD") : TEXT("ALIVE");
	const FString Final   = Row.FinalRank > 0 ? FString::Printf(TEXT("Final=%d"), Row.FinalRank)
											  : TEXT("-");
	const FString Live    = Row.LiveRank > 0  ? FString::Printf(TEXT("Live=%d"), Row.LiveRank)
											  : TEXT("-");
	const FString IdShort = Row.LocalUserId.IsEmpty() ? TEXT("-")
													  : (Row.LocalUserId.Len() > 10
														 ? Row.LocalUserId.Left(10) + TEXT("…")
														 : Row.LocalUserId);

	return FString::Printf(
		TEXT("[%02d] %-5s | HP=%6.1f | %-8s | %-8s | Name=%s | Id=%s"),
		SlotRank,
		*Status,
		Row.Hp,
		*Final,
		*Live,
		*IdShort
	);
}


void APCCombatGameState::DebugPrintLeaderboard(bool bToScreen, float ScreenSeconds)
{
	if (HasAuthority())
	{
		// 서버에서 직접 출력 or 멀티캐스트
		// 라인 구성
		TArray<FString> Lines;
		Lines.Reserve(Leaderboard.Num() + 2);

		// 요약 헤더
		int32 Alive = 0;
		for (const auto& R : Leaderboard)
		{
			if (R.LocalUserId.Len() > 0 && R.FinalRank == 0) ++Alive;
		}
		Lines.Add(FString::Printf(TEXT("=== Leaderboard (%d players, Alive=%d) ==="), Leaderboard.Num(), Alive));

		// 각 슬롯
		for (int32 i = 0; i < Leaderboard.Num(); ++i)
		{
			Lines.Add(FormatRowLine(i, Leaderboard[i]));
		}

		// 로그 출력
		for (const FString& L : Lines)
		{
			UE_LOG(LogTemp, Log, TEXT("%s"), *L);
		}

		if (bToScreen)
		{
			Multicast_DebugPrintToScreen(Lines, ScreenSeconds);
		}
	}
	else
	{
		// 클라에서 호출되면 서버에게 요청
		Server_DebugPrintLeaderboard(bToScreen, ScreenSeconds);
	}
}

void APCCombatGameState::Server_DebugPrintLeaderboard_Implementation(bool bToScreen, float ScreenSeconds)
{
	DebugPrintLeaderboard(bToScreen, ScreenSeconds);
}

void APCCombatGameState::Multicast_DebugPrintToScreen_Implementation(const TArray<FString>& Lines, float ScreenSeconds)
{
	if (!GEngine) return;

	// 첫 줄은 굵게/길게
	if (Lines.Num() > 0)
	{
		GEngine->AddOnScreenDebugMessage(
			/*Key*/-1, ScreenSeconds, FColor::Cyan, Lines[0]
		);
	}

	for (int32 i = 1; i < Lines.Num(); ++i)
	{
		GEngine->AddOnScreenDebugMessage(
			/*Key*/-1, ScreenSeconds, FColor::Green, Lines[i]
		);
	}
}
