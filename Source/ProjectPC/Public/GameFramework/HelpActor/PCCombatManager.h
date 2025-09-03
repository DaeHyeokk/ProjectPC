// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PCCombatManager.generated.h"

class APCPlayerState;
class UPCTileManager;
class APCCombatBoard;
class APCHeroUnitCharacter;

USTRUCT()
struct FCombatManager_FieldSlot
{
	GENERATED_BODY()
	UPROPERTY()
	int32 Col = 0;
	UPROPERTY()
	int32 Row = 0;
	UPROPERTY()
	TWeakObjectPtr<APCHeroUnitCharacter> Unit;
};

USTRUCT()
struct FCombatManager_BenchSlot
{
	GENERATED_BODY()
	UPROPERTY()
	int32 Index = 0;
	UPROPERTY()
	TWeakObjectPtr<APCHeroUnitCharacter> Unit;
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
	TArray<TWeakObjectPtr<APCHeroUnitCharacter>> MovedUnits;
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

	// 카메라 세팅
	UFUNCTION()
	void Server_TravelFocusCamera(APCPlayerState* PCPlayerStateA, int32 BoardSeatIndexA, APCPlayerState* PCPlayerStateB, int32 BoardSeatIndexB, float Blend);

	UFUNCTION()
	void Server_ReturnFocusCamera(APCPlayerState* PCPlayerStateA, int32 BoardSeatIndexA, APCPlayerState* PCPlayerStateB, int32 BoardSeatIndexB, float Blend);
	

private:
	bool IsAuthority() const { return GetLocalRole() == ROLE_Authority; }

	static APCCombatBoard* FindBoardBySeatIndex(UWorld* World, int32 SeatIndex);
	static void TakeSnapshot(APCCombatBoard* Board, FCombatManager_BoardSnapShot& BoardSnapShot);
	static void RestoreSnapshot(const FCombatManager_BoardSnapShot& Snap);
	static bool RemoveUnitFromAny(UPCTileManager* TileManager, APCHeroUnitCharacter* Unit);

public:	
	

	
	
};
