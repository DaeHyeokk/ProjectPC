// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseGameplayTags.h"
#include "GameplayTagAssetInterface.h"
#include "GameFramework/GameStateBase.h"
#include "DataAsset/FrameWork/PCStageData.h"
#include "DataAsset/Projectile/PCDataAsset_ProjectilePoolData.h"
#include "GameFramework/PlayerState/PCLevelMaxXPData.h"
#include "PCCombatGameState.generated.h"

class APCItemCapsule;
class UAbilitySystemComponent;
class APCUnitCombatTextActor;
class UPCTileManager;
class APCCombatBoard;
class UPCShopManager;
class APCPlayerState;

USTRUCT(BlueprintType)
struct FSpawnSubsystemConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category="Spawner|Registry")
	TSoftObjectPtr<class UPCDataAsset_UnitDefinitionReg> Registry = nullptr;

	UPROPERTY(EditAnywhere, Category="Spawner|Defaults")
	TSoftClassPtr<class APCCreepUnitCharacter> DefaultCreepClass;
	
	UPROPERTY(EditAnywhere, Category="Spawner|Defaults")
	TSoftClassPtr<class APCAppearanceChangedHeroCharacter> DefaultAppearanceChangedHeroClass;
	
	UPROPERTY(EditAnywhere, Category="Spawner|Defaults")
	TSoftClassPtr<class APCAppearanceFixedHeroCharacter> DefaultAppearanceFixedHeroClass;

	UPROPERTY(EditAnywhere, Category="Spawner|Defaults")
	TSoftClassPtr<class UPCUnitStatusBarWidget> CreepStatusBarWidgetClass;
	
	UPROPERTY(EditAnywhere, Category="Spawner|Defaults")
	TSoftClassPtr<class UPCHeroStatusBarWidget> HeroStatusBarWidgetClass;
	
	// == 공통 AI Controller (전 유닛 공유) ==
	UPROPERTY(EditAnywhere, Category="Spawner|AI")
	TSoftClassPtr<class APCUnitAIController> DefaultAIControllerClass;

	UPROPERTY(EditAnywhere, Category="Spawner|PreviewHero")
	TSoftClassPtr<class APCPreviewHeroActor> DefaultPreviewHeroClass;
	
	UPROPERTY(EditAnywhere, Category="Spawner|CarouselHero")
	TSoftClassPtr<class APCCarouselHeroCharacter> DefaultCarouselHeroClass;

	UPROPERTY(EditAnywhere, Category="Spawner|OutlineMaterial")
	TSoftObjectPtr<UMaterialInterface> DefaultOutlineMaterial;
};

USTRUCT(BlueprintType)
struct FStageRuntimeState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int32 FloatIndex = -1;

	UPROPERTY(BlueprintReadOnly)
	int32 StageIdx = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 RoundIdx = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 StepIdxInRound = 0;

	UPROPERTY(BlueprintReadOnly)
	EPCStageType Stage = EPCStageType::Setup;

	UPROPERTY(BlueprintReadOnly)
	float Duration = 0.f;

	UPROPERTY(BlueprintReadOnly)
	float ServerStartTime = 0.f;

	UPROPERTY(BlueprintReadOnly)
	float ServerEndTime = 0.f;
};

/** 메인 위젯에 그대로 나열할 행(1등=Index 0, N등=Index N-1) */
USTRUCT(BlueprintType)
struct FPlayerStandingRow
{
	GENERATED_BODY()

	/** 플레이어ID 식별자 */
	UPROPERTY(BlueprintReadOnly)
	FString LocalUserId;

	// 카메라 전환용 SeatIndex
	UPROPERTY(BlueprintReadOnly)
	int32 PlayerSeatIndex = -1;
	
	/** 최신 HP (실시간) */
	UPROPERTY(BlueprintReadOnly)
	float Hp = 0.f;

	/** 사망 여부 */
	UPROPERTY(BlueprintReadOnly)
	bool bEliminated = false;

	/** 실시간 생존자 랭크(1부터). 사망자는 0 */
	UPROPERTY(BlueprintReadOnly)
	int32 LiveRank = 0;

	/** 확정 최종 등수(사망 순간 할당. 마지막 생존자는 1) */
	UPROPERTY(BlueprintReadOnly)
	int32 FinalRank = 0;

	/** 리스트 안정화용: 최초 관측 순서(사망 섹션 정렬에는 사용 X, 필요 시 참고) */
	UPROPERTY(BlueprintReadOnly)
	int32 StableOrder = 0;

	/** HP 동률 안정화를 위한 마지막 변경 시각(서버시간) */
	UPROPERTY(BlueprintReadOnly)
	float LastChangeTime = 0.f;

	UPROPERTY(BlueprintReadOnly)
	FGameplayTag CharacterTag;
};

UENUM(BlueprintType)
enum class ERoundResult : uint8
{
	None UMETA(DisplayName = "None"),
	Victory UMETA(DisplayName = "Victory"),
	Defeat UMETA(DisplayName = "Defeat"),
	Draw UMETA(DisplayName = "Draw")
};

// GameStateWidget 표시용 델리게이트
DECLARE_MULTICAST_DELEGATE(FOnStageRuntimeChanged);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnGameStateTagChanged, const FGameplayTag&);
DECLARE_MULTICAST_DELEGATE(FOnRoundsLayoutChanged);

// Leaderboard 맵 델리게이트
using FLeaderBoardMap = TMap<FString, FPlayerStandingRow>;
DECLARE_MULTICAST_DELEGATE_OneParam(FOnLeaderboardMapUpdatedNative, const FLeaderBoardMap&);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnLeaderboardFindPlayerState, const TArray<APCPlayerState*>&);
DECLARE_MULTICAST_DELEGATE(FOnLeaderBoardReadyNative);

// Carousel 전용 델리게이트
DECLARE_MULTICAST_DELEGATE(FOnCarouselGetScheduleChanged);
/**
 * 
 */
UCLASS()
class PROJECTPC_API APCCombatGameState : public AGameStateBase, public IGameplayTagAssetInterface
{
	GENERATED_BODY()

public:
	APCCombatGameState();
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;

#pragma region GameLogic
	
	// 전체 게임 로직 관련 코드
public:

	// SeatIndex -> Board
	UPROPERTY(Replicated)
	TArray<APCCombatBoard*> SeatToBoard;

	UPROPERTY(Replicated)
	bool bBoardMappingComplete = false;

	UFUNCTION()
	void BuildSeatToBoardMap(const TArray<APCCombatBoard*>& Boards);

	UFUNCTION(BLueprintPure)
	APCCombatBoard* GetBoardBySeat(int32 PlayerSeatIndex) const;

	// GameStageState
	UFUNCTION(BlueprintCallable, Category = "Stage")
	void SetStageRunTime(const FStageRuntimeState& NewState);

	UFUNCTION(BlueprintPure, Category = "Stage")
	const FStageRuntimeState& GetStageRunTime() const { return StageRuntimeState;}

	UPCTileManager* GetBattleTileManagerForSeat(int32 SeatIdx) const;
	APCCombatBoard* GetBattleBoardForSeat(int32 SeatIdx) const;

	


#pragma endregion GameLogic

#pragma region UI

public:

	// UI에서 바인딩할 값들
	UFUNCTION(BlueprintPure)
	float GetStageRemainingSeconds() const;

	UFUNCTION(BlueprintPure)
	float GetStageProgress() const;

	UFUNCTION(BlueprintPure)
	FString GetStageLabelString() const;

	UFUNCTION(BlueprintPure)
	EPCStageType GetCurrentStageType() const;

	// UI 헬퍼
	UFUNCTION(BlueprintPure, Category = "Stage")
	FORCEINLINE int32 GetStageIndex() const { return StageRuntimeState.StageIdx;}

	UFUNCTION(BlueprintPure, Category = "Stage")
	FORCEINLINE int32 GetRoundIndex() const { return StageRuntimeState.RoundIdx;}

	// Round Layout
	void SetRoundsPerStage(const TArray<int32>& InCounts);
	void SetRoundMajorsFlat(const TArray<FGameplayTag>& InFlatMajors);

	UFUNCTION(BlueprintPure, Category = "Stage|LayOut")
	int32 GetNumStages() const { return RoundsPerStage.Num();}

	UFUNCTION(BlueprintPure, Category = "Stage|LayOut")
	int32 GetNumRoundsInStage(int32 StageIdx) const;

	UFUNCTION(BlueprintPure, Category = "Stage|LayOut")
	FGameplayTag GetMajorStageForRound(int32 StageIdx, int32 RoundIdx) const;

	UFUNCTION(BlueprintPure, Category = "Stage|LayOut")
	FGameplayTag GetPvETagForRound(int32 StageIdx, int32 RoundIdx) const;

	UFUNCTION(BlueprintPure, Category = "Stage|LayOut")
	int32 StagesStartFlatIndex(int32 StageIdx) const;

	
	UPROPERTY(ReplicatedUsing=OnRep_RoundsLayout, BlueprintReadOnly, Category = "Stage|Layout")
	TArray<FGameplayTag> RoundPvETagFlat;
		
	
	FOnStageRuntimeChanged OnStageRuntimeChanged;
	FOnGameStateTagChanged OnGameStateTagChanged;
	FOnRoundsLayoutChanged OnRoundsLayoutChanged;
	
	UPROPERTY(ReplicatedUsing=OnRep_StageRunTime, BlueprintReadOnly, Category = "Stage")
	FStageRuntimeState StageRuntimeState;

	// 전투결과 반영
	// 클라 로컬 캐시
	UPROPERTY(ReplicatedUsing=OnRep_RoundResult)
	TArray<ERoundResult> SeatRoundResult;

	UFUNCTION()
	void OnRep_RoundResult();

	void ApplyRoundResultForSeat(int32 SeatIdx, int32 StageIdx, int32 RoundIdx, ERoundResult Result);
	
	UFUNCTION(BlueprintPure)
	int32 GetMySeatIndex() const;
	
	ERoundResult GetRoundResultForSeat(int32 SeatIdx, int32 StageIdx, int32 RoundIdx) const;

	// UI 헬퍼
	UFUNCTION(BlueprintPure)
	bool WasRoundVictory(int32 StageIdx, int32 RoundIdx) const;

	UFUNCTION(BlueprintPure)
	bool WasRoundDefeat(int32 StageIdx, int32 RoundIdx) const;

	UFUNCTION(BlueprintPure)
	bool WasRoundDraw(int32 StageIdx, int32 RoundIdx) const;
	
	
protected:
	
	UFUNCTION()
	void OnRep_StageRunTime();

	// 라운드 개수
	UPROPERTY(ReplicatedUsing=OnRep_RoundsLayout, BlueprintReadOnly, Category = "Stage|Layout")
	TArray<int32> RoundsPerStage;

	UPROPERTY(ReplicatedUsing=OnRep_RoundsLayout, BlueprintReadOnly, Category = "Stage|Layout")
	TArray<FGameplayTag> RoundMajorFlat;

	UFUNCTION()
	void OnRep_RoundsLayout();

	int32 TotalRoundsFloat() const;

	
#pragma endregion UI
	
#pragma region Shop
	
protected:
	UPROPERTY(VisibleDefaultsOnly, Category = "ShopManager")
	UPCShopManager* ShopManager;

public:
	// GameState 생성자에서 생성하므로, ShopManager가 nullptr인 경우 바로 크래시
	// => 즉 GetShopManager()가 nullptr을 반환할 일은 없음
	FORCEINLINE UPCShopManager* GetShopManager() const { return ShopManager; }

#pragma endregion Shop

#pragma region Attribute
	
protected:
	// 플레이어 레벨 별 MaxXP 정보가 담긴 DataTable
	UPROPERTY(EditAnywhere, Category = "DataTable|Player")
	TObjectPtr<UDataTable> LevelMaxXPDataTable;

private:
	// 실제로 DataTable에서 가져온 정보를 저장할 배열
	TArray<FPCLevelMaxXPData> LevelMaxXPDataList;

public:
	int32 GetMaxXP(int32 PlayerLevel) const;

#pragma endregion Attribute

#pragma region Unit
	
private:
	UPROPERTY(EditAnywhere, Category="Unit Spawner Data")
	FSpawnSubsystemConfig SpawnConfig;

public:
	FORCEINLINE const FSpawnSubsystemConfig& GetSpawnConfig() const { return SpawnConfig; }
	
#pragma endregion Unit

#pragma region Combat
	
public:
	void SetGameStateTag(const FGameplayTag& InGameStateTag);
	UFUNCTION(BlueprintPure)
	const FGameplayTag& GetGameStateTag() const { return GameStateTag; }
	bool IsCombatActive() const { return GameStateTag.MatchesTag(GameStateTags::Game_State_Combat); }

	bool bIsbattle() const { return GameStateTag == GameStateTags::Game_State_Combat_Preparation || GameStateTag == GameStateTags::Game_State_Combat_Active; }
	
protected:
	UPROPERTY(ReplicatedUsing=OnRep_GameStateTag)
	FGameplayTag GameStateTag;

	UFUNCTION()
	void OnRep_GameStateTag();
	
	// ==== 전투 시스템 | BT 관련 ====
protected:
	//BT Decorator에서 Game State Tag 정보 참조하기 위해
	//IGameplayTagAssetInterface 상속 받아서 오버라이드한 함수
	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;
	virtual bool HasMatchingGameplayTag(FGameplayTag TagToCheck) const override;
	virtual bool HasAllMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const override;
	virtual bool HasAnyMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const override;
	
	// TEST CODE //
public:
	UFUNCTION(BlueprintCallable)
	void Test_StartCombat() { SetGameStateTag(GameStateTags::Game_State_Combat_Active); }

#pragma endregion Combat
	
#pragma region ObjectPool

protected:
	UPROPERTY(EditDefaultsOnly, Category = "ObjectPool")
	TObjectPtr<UPCDataAsset_ProjectilePoolData> ProjectilePoolData;

	UPROPERTY(EditDefaultsOnly, Category="ObjectPool")
	TSoftClassPtr<APCUnitCombatTextActor> CombatTextClass;
	
#pragma endregion ObjectPool
	
#pragma region TemplateFunc
	
private:
	// DataTable을 읽어 아웃파라미터로 TArray에 값을 넘기는 템플릿 함수
	template<typename T>
	void LoadDataTable(UDataTable* DataTable, TArray<T>& OutDataList, const FString& Context)
	{
		if (DataTable == nullptr) return;
		OutDataList.Reset();

		TArray<T*> RowPtrs;
		DataTable->GetAllRows(Context, RowPtrs);

		// DataTable의 Row수만큼 메모리 미리 확보
		OutDataList.Reserve(RowPtrs.Num());
		for (const auto Row : RowPtrs)
		{
			if (Row)
			{
				OutDataList.Add(*Row);
			}
		}
	}

#pragma endregion TemplateFunc

#pragma region Ranking

public:

	FOnLeaderboardMapUpdatedNative OnLeaderboardMapUpdated;
	FOnLeaderBoardReadyNative OnLeaderBoardReady;
	FOnLeaderboardFindPlayerState FindPlayerState;
	
	// UI에 뿌릴 최종 배열
	UPROPERTY(ReplicatedUsing=OnRep_LeaderBoard, BlueprintReadOnly, Category = "Ranking")
	TArray<FPlayerStandingRow> Leaderboard;

	// LocalUserId -> 확정 최종 등수
	UPROPERTY(BlueprintReadOnly, Category = "Ranking")
	TMap<FString, int32> FinalRanks;

	bool IsLeaderboardReady() const { return bLeaderBoardReady;}

	// 최초 도착 감지용
	bool bLeaderBoardReady = false;

	// 어트리뷰트 바인딩 함수
	UFUNCTION(BlueprintCallable, Category = "Ranking||Bind")
	void BindAllPlayerHP();

	UFUNCTION(BlueprintCallable, Category = "Ranking||Bind")
	void BindOnePlayerHpDelegate(APCPlayerState* PCPlayerState);

	// 자신의 최종 등수 반환 함수
	UFUNCTION(BlueprintCallable, Category = "Ranking")
	int32 AssignFinalRankOnDeathById(const FString& LocalUserId);

	// 편의용 state 버전
	UFUNCTION(BlueprintCallable, Category = "Ranking")
	int32 AssignFinalRankOnDeathByPS(APCPlayerState* PCPlayerState);

	UFUNCTION(BlueprintPure, Category = "Ranking")
	int32 GetFinalRankFor(const FString& LocalUserId) const;

	TMap<FString, FPlayerStandingRow> GetLeaderBoardMap() const { return CachedLeaderboardMap;}

	void RebuildAndReplicatedLeaderboard();

protected:

	virtual void RemovePlayerState(APlayerState* PlayerState) override;

	// ASC AttributeChangeDelegate
	void OnHpChanged_Server(APCPlayerState* PCPlayerState, float NewHp);
	void OnEliminated_Server(APCPlayerState* PCPlayerState);

	// 리더보드 재구성, 마지막 1인 1등 처리
	void TryFinalizeLastSurvivor();

	// 로컬 UserID로 플레이어스테이트 찾기
	UFUNCTION()
	APCPlayerState* FindPlayerStateByUserId(const FString& LocalUserId) const;

	// 순위대로 PS 뽑기
	UFUNCTION(BlueprintCallable, Category = "Leaderboard")
	void GetPlayerStatesOrdered(TArray<APCPlayerState*>& OutPlayerStates) const;

	
	// 위젯 갱신
	UFUNCTION()
	void OnRep_Leaderboard();
	
	FLeaderBoardMap CachedLeaderBoardMap;

	UPROPERTY()
	TMap<FString, FPlayerStandingRow> CachedLeaderboardMap;

	UAbilitySystemComponent* ResolveASC(APCPlayerState* PCPlayerState) const;

	void BroadCastLeaderboardMap();

	UPROPERTY(Replicated)
	TArray<APCPlayerState*> FindPlayerStates;

public:
	const TArray<APCPlayerState*>& GetPlayerStates() { return FindPlayerStates; };

private:

	/** 서버 캐시들 (키 = LocalUserId) */
	TMap<FString, float> HpCache;             // 최신 HP
	TMap<FString, float> LastChangeTimeCache; // 마지막 HP 변경시간(서버)
	TMap<FString, int32> StableOrderCache;    // 최초 관측 순서
	TSet<FString>        EliminatedSet;       // 사망자 집합
	
	int32 AliveCount = 0;
	int32 StableOrderCounter = 0;

	/** ASC 바인딩 핸들 관리 */
	TMap<TWeakObjectPtr<UAbilitySystemComponent>, FDelegateHandle> HpDelegateHandles;


public:
	// ==== Debug ====
	UFUNCTION(BlueprintCallable, Exec, Category="Rank|Debug")
	void DebugPrintLeaderboard(bool bToScreen = true, float ScreenSeconds = 5.f);

	UFUNCTION(Server, Reliable)
	void Server_DebugPrintLeaderboard(bool bToScreen, float ScreenSeconds);

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_DebugPrintToScreen(const TArray<FString>& Lines, float ScreenSeconds);
	
#pragma endregion Ranking

#pragma region Item

protected:
	// 아이템 데이터가 저장된 DataTable
	UPROPERTY(EditAnywhere, Category = "DataTable|Item")
	TObjectPtr<UDataTable> ItemDataTable;

	// 아이템 조합 데이터가 저장된 DataTable
	UPROPERTY(EditAnywhere, Category = "DataTable|Item")
	TObjectPtr<UDataTable> ItemCombineDataTable;

	// 아이템 캡슐 블루프린트 클래스
	UPROPERTY(EditAnywhere, Category = "ItemCapsuleClass")
	TSubclassOf<APCItemCapsule> ItemCapsuleClass;
	
#pragma endregion Item


};
