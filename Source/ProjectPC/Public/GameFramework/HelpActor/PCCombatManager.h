// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/Unit/PCBaseUnitCharacter.h"
#include "GameFramework/Actor.h"
#include "PCCombatManager.generated.h"

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

USTRUCT()
struct FCombatManager_BenchSlot
{
	GENERATED_BODY()
	UPROPERTY()
	int32 Index = 0;
	UPROPERTY()
	TWeakObjectPtr<APCBaseUnitCharacter> Unit;
};

USTRUCT()
struct FCombatManager_BoardSnapShot
{
	GENERATED_BODY()
	UPROPERTY()
	TWeakObjectPtr<APCCombatBoard> CombatBoard;
	UPROPERTY()
	TWeakObjectPtr<UPCTileManager> Tile;
	UPROPERTY()
	TArray<FCombatManager_FieldSlot> Field;
	UPROPERTY()
	TArray<FCombatManager_BenchSlot> Bench;

	void Reset()
	{
		CombatBoard = nullptr;
		Tile = nullptr;
		Field.Reset();
		Bench.Reset();
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
	FCombatManager_BoardSnapShot HostSnapShot;
	UPROPERTY()
	FCombatManager_BoardSnapShot GuestSnapShot;
	UPROPERTY()
	TArray<TWeakObjectPtr<APCBaseUnitCharacter>> MovedUnits;

	// 전투 중 새로 생성된 유닛(구매/합성 등) 기록 : Seat -> Units
	TMap<int32, TArray<TWeakObjectPtr<APCBaseUnitCharacter>>> NewUnitDuringBattle;

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

	// 현재 라운드 페어링
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Combat|State")
	TArray<FCombatManager_Pair> Pairs;

	// 결과 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "Combat|State")
	FOnCombatPairResult OnCombatPairResult;

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
	

private:
	bool IsAuthority() const { return GetLocalRole() == ROLE_Authority; }

	// 전투 스냅샷
	static void TakeSnapshot(APCCombatBoard* Board, FCombatManager_BoardSnapShot& BoardSnapShot);
	static void RestoreSnapshot(const FCombatManager_BoardSnapShot& Snap);
	static bool RemoveUnitFromAny(UPCTileManager* TileManager, APCBaseUnitCharacter* Unit);

	// 좌석 기반 조회 함수
	UFUNCTION(BlueprintCallable)
	APCCombatBoard* FindBoardBySeatIndex(UWorld* World, int32 SeatIndex);
	
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


	// 전투중 구매
	UFUNCTION()
	void OnUnitSpawnedDuringBattle(APCBaseUnitCharacter* Unit, int32 SeatIndex);
	
public:
	
	int32 FindRunningPairIndexBySeat(int32 SeatIndex) const;
	int32 FindFirstFreeBenchIndex(UPCTileManager* TM, bool bEnemySide) const;

	// 데미지 관련
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Round|Damage")
	FGameplayTag DamageEventTag;
	
	UPROPERTY(EditDefaultsOnly, Category = "Round|Damage")
	UDataTable* StageDamageTable;

	UPROPERTY(EditDefaultsOnly, Category = "Round|Damage")
	bool bClampToLastRow = true;


protected:
	int32 GetCurrentStageIndex() const;
	int32 GetStageBaseDamageFromDT(int32 StageIdx) const;
	int32 GetStageBaseDamageDefault(int32 StageIdx) const;
	
	
};
