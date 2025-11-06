// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PCPlayerBoard.h"
#include "Character/Unit/PCBaseUnitCharacter.h"
#include "GameFramework/Actor.h"
#include "PCCombatManager.generated.h"

struct FPlayerBoardSnapshot;
enum class ETileFacing : uint8;
class APCPlayerBoard;
class APCPlayerCharacter;
class APCCombatPlayerController;
class APCBaseUnitCharacter;
class APCPlayerState;
class UPCTileManager;
class APCCombatBoard;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnCombatPairResult, int32, WinnerPlayer, int32, LoserPlayer, int32, HostAlive, int32, GuestAlive);

USTRUCT()
struct FCombatManager_FieldSlot
{
	GENERATED_BODY()
	
	UPROPERTY()
	int32 Col = 0;
	UPROPERTY()
	int32 Row = 0;
	UPROPERTY()
	TWeakObjectPtr<APCBaseUnitCharacter> Unit;
};


USTRUCT(BlueprintType)
struct FBoardFieldSnapShot
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TWeakObjectPtr<APCPlayerBoard> PlayerBoard;
	
	UPROPERTY(EditAnywhere)
	TArray<FCombatManager_FieldSlot> Field; // (Col, Row, Unit)만 저장

	void Reset()
	{
		PlayerBoard = nullptr;
		Field.Reset();
	}
};

USTRUCT(BlueprintType)
struct FCombatManager_Pair
{
	GENERATED_BODY()
	UPROPERTY()
	TWeakObjectPtr<APCCombatBoard> Host;
	UPROPERTY()
	TWeakObjectPtr<APCCombatBoard> Guest;

	UPROPERTY()
	FBoardFieldSnapShot HostSnapShot;
	UPROPERTY()
	FBoardFieldSnapShot GuestSnapShot;
	UPROPERTY()
	FPlayerBoardSnapshot PvESnapShot;

	UPROPERTY()
	bool bIsClone = false;

	UPROPERTY()
	int32 CloneSourceSeat = INDEX_NONE;

	UPROPERTY()
	TArray<TWeakObjectPtr<APCBaseUnitCharacter>> CloneUnits;
	
	// PVE 지원
	UPROPERTY()
	bool bIsPvE = false;
	UPROPERTY()
	TSet<TWeakObjectPtr<APCBaseUnitCharacter>> PvECreeps;
	
	// 전투상태
	UPROPERTY()
	int32 HostAlive = 0;

	UPROPERTY()
	int32 GuestAlive = 0;

	UPROPERTY()
	bool bRunning = false;

	// 중복 집계 방지
	UPROPERTY()
	TSet<TWeakObjectPtr<APCBaseUnitCharacter>> DeadUnits;

	void ResetRuntime()
	{
		PvECreeps.Reset();
		DeadUnits.Reset();
		HostAlive = 0;
		GuestAlive = 0;
		bRunning = false;
	}
};




UCLASS()
class PROJECTPC_API APCCombatManager : public AActor
{
	GENERATED_BODY()
	
public:	
	
	APCCombatManager();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	bool bIncludeBench = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	bool bMirrorRows = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	bool bMirrorCols = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Match")
	int32 RandomSeed = 12345;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Match")
	bool bReseedEveryRound = true;

	// PvE 스폰용 기본 크립 클래스
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|PvE")
	TSubclassOf<APCBaseUnitCharacter> DefaultCreepClass;

	// 현재 라운드 페어링
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Combat|State")
	TArray<FCombatManager_Pair> Pairs;

	// 결과 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "Combat|State")
	FOnCombatPairResult OnCombatPairResult;

	// PvP 흐름
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void BuildRandomPairs();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void StartAllBattle();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void FinishAllBattle();

	
	// 플레이어 전체 이동

	UPROPERTY(BlueprintReadOnly, Category = "Combat|Travel")
	FRotator GuestRotation = FRotator(0.f, 180.f, 0.f);

	UFUNCTION(BlueprintCallable, Category = "Combat|Travel")
	void TravelPlayersForAllPairs(float Blend = 0.35f);

	UFUNCTION(BlueprintCallable, Category = "Combat|Travel")
	void ReturnPlayersForAllPairs(float Blend = 0.35f);

	// 특정 페어만 이동 / 복귀
	UFUNCTION(BlueprintCallable, Category = "Combat|Travel")
	void TravelPlayersForPair(int32 PairIndex, float Blend = 0.35f);

	UFUNCTION(BlueprintCallable, Category = "Combat|Travel")
	void ReturnPlayersForPair(int32 PairIndex, float Blend = 0.35f);
	

	// 승리체크
	void CheckVictory();
	
	// ===== PvE: 크립 스폰/원복 =====
	// 현재 스테이지/라운드를 읽고 BuildCreepPoints를 사용해 좌표 생성 → 크립 스폰/배치/바인딩
	UFUNCTION(BlueprintCallable, Category="Combat|PvE")
	int32 StartPvEBattleForSeat(int32 HostSeatIndex);

	// 크립 정리 + Host 스냅샷 원복 + 언바인드
	UFUNCTION(BlueprintCallable, Category="Combat|PvE")
	void FinishPvEBattleForSeat(int32 HostSeatIndex);

	UFUNCTION(BLueprintCallable, Category="Combat|PvE")
	void FinishAllPve();
	
private:
	bool IsAuthority() const { return GetLocalRole() == ROLE_Authority; }
	
	UFUNCTION()
	void TakeFieldSnapShot(APCPlayerBoard* PlayerBoard, FBoardFieldSnapShot& Out);
	
	UFUNCTION()
	void RestoreFieldSnapShot(const FBoardFieldSnapShot& Snap);
	
	static bool RemoveUnitFromAny(UPCTileManager* TileManager, APCBaseUnitCharacter* Unit);
	
	// 좌석 기반 조회 함수
	UFUNCTION(BlueprintCallable)
	APCCombatBoard* FindBoardBySeatIndex(UWorld* World, int32 SeatIndex);

	UFUNCTION(BlueprintCallable)
	APCPlayerBoard* FindPlayerBoardBySeat(int32 SeatIndex) const;
	
	APCPlayerState* FindPlayerStateBySeat(int32 SeatIndex) const;
	APCCombatPlayerController* FindPlayerController(int32 SeatIndex) const;
	APawn* FindPawnBySeat(int32 SeatIndex) const;

	// 이동 및 카메라 유틸 함수
	void TeleportPlayerToTransform(APawn* PlayerCharacter, const FTransform& T) const;
	void FocusCameraToBoard(int32 ViewerSeatIdx, int32 BoardSeatIdx, bool bIsBattle, float Blend);

	// 유닛 -> 페어 인덱스 매핑 (죽음 이벤트 라우팅)
	TMap<TWeakObjectPtr<APCBaseUnitCharacter>, int32> UnitToPairIndex;

	// 전투 바인딩 / 판정
	void BindUnitOnBoardForPair(int32 PairIndex);
	void UnbindAllForPair(int32 PairIndex);
	void CountAliveOnHostBoardForPair(int32 PairIndex);
	
	UFUNCTION()
	void OnAnyUnitDied(APCBaseUnitCharacter* Unit);
	
	void CheckPairVictory(int32 PairIndex);
	
	void ResolvePairResult(int32 PairIndex, bool bHostWon);

	// Clone PvP
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void BuildCloneForHost(int32 PairIndex, int32 DonorSeat);

	// Clone 파괴 헬퍼
	void DestroyCloneForPair(int32 PairIndex, bool bRemoveFromTM = true);

	// 전투시 전투 필드로 이동
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void PlacePlayerBoardToTM(APCPlayerBoard* PlayerBoard, UPCTileManager* TM, bool MirrorRows, bool MirrorCols, ETileFacing Facing);


public:
	
	int32 FindRunningPairIndexBySeat(int32 SeatIndex) const;
	// 타임아웃 핸들러
	void HandleBattleFinished();

	// 데미지 관련
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Round|Damage")
	FGameplayTag DamageEventTag;
	
	UPROPERTY(EditDefaultsOnly, Category = "Round|Damage")
	UDataTable* StageDamageTable;

	UPROPERTY(EditDefaultsOnly, Category = "Round|Damage")
	bool bClampToLastRow = true;


protected:
	

	// 골드 디스플레이 델리게이트
	FDelegateHandle GoldDisplayHandle;

	// 전투 진행도 체크 함수
	void NotifyAllBattleFinished();



	// 무승부 처리
	void ResolvePairDraw(int32 PairIndex);
	
	int32 GetCurrentStageIndex() const;
	int32 GetStageBaseDamageFromDT(int32 StageIdx) const;
	int32 GetStageBaseDamageDefault(int32 StageIdx) const;
	
	// StageOne/RoundOne(1-기준) 조회
	bool GetCurrentStageRoundOne(int32& OutStageOne, int32& OutRoundOne) const;

	// ===== PvE 유틸 =====
	static constexpr int32 CREEP_TEAM_BASE = 50;
	static int32 GetCreepTeamIndexForBoard(const APCCombatBoard* Board) { return Board ? (Board->BoardSeatIndex + CREEP_TEAM_BASE) : CREEP_TEAM_BASE; }

	// Stage/Round 기반 크립 태그/레벨 (필요 시 프로젝트 태그로 수정)
	FGameplayTag GetCreepTagForStageRound(int32 StageOne, int32 RoundOne) const;
	int32        GetCreepLevelForStageRound(int32 StageOne, int32 RoundOne) const;

	// (Y,X) 입력을 받아 해당 자리에 두거나, 주변으로 탐색해서 배치
	bool PlaceOrNearest(UPCTileManager* TM, int32 Y, int32 X, APCBaseUnitCharacter* Creep) const;

	// GameMode와 동일하게 Tag/Team/Level 기반 스폰 + 보드 배치
	APCBaseUnitCharacter* SpawnCreepAt(APCCombatBoard* Board, int32 StageOne, int32 RoundOne, const FIntPoint& YX) const;
};
