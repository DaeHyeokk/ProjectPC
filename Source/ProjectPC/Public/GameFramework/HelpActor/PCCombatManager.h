// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PCCombatManager.generated.h"

class APCPlayerCharacter;
class APCCombatPlayerController;
class APCBaseUnitCharacter;
class APCPlayerState;
class UPCTileManager;
class APCCombatBoard;

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
	FCombatManager_BoardSnapShot GuestSnapShot;
	UPROPERTY()
	TArray<TWeakObjectPtr<APCBaseUnitCharacter>> MovedUnits;
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

	// 현재 라운드 페어링
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Combat|State")
	TArray<FCombatManager_Pair> Pairs;

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void BuildRandomPairs();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void StartAllBattle();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void FinishAllBattle();

	// 플레이어 전체 이동

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

	static APCCombatBoard* FindBoardBySeatIndex(UWorld* World, int32 SeatIndex);
	static void TakeSnapshot(APCCombatBoard* Board, FCombatManager_BoardSnapShot& BoardSnapShot);
	static void RestoreSnapshot(const FCombatManager_BoardSnapShot& Snap);
	static bool RemoveUnitFromAny(UPCTileManager* TileManager, APCBaseUnitCharacter* Unit);

	// 좌석 기반 조회 함수
	APCPlayerState* FindPlayerStateBySeat(int32 SeatIndex) const;
	APCCombatPlayerController* FindPlayerController(int32 SeatIndex) const;
	APawn* FindPawnBySeat(int32 SeatIndex) const;

	// 이동 및 카메라 유틸 함수
	void TeleportPlayerToTransform(APawn* PlayerCharacter, const FTransform& T) const;
	void FocusCameraToBoard(int32 ViewerSeatIdx, int32 BoardSeatIdx, bool bIsBattle, float Blend);
	

	
	
};
