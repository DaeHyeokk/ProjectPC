// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PCTileManager.generated.h"

class APCCombatBoard;
class APCBaseUnitCharacter;

USTRUCT(BlueprintType)
struct FTile
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FIntPoint UnitIntPoint;
	
	UPROPERTY(BlueprintReadWrite)
	FVector Position = FVector::ZeroVector;
	
	UPROPERTY(BlueprintReadWrite)
	APCBaseUnitCharacter* Unit = nullptr;

	UPROPERTY(BlueprintReadWrite)
	bool bIsField = true;
	
	bool IsEmpty() const { return Unit == nullptr; }
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTPC_API UPCTileManager : public UActorComponent
{
	GENERATED_BODY()

public:	
	
	UPCTileManager();
	
	
	// 필드 타일 크기 ( 8 * 7 )
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Field")
	int32 Cols = 8;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Field")
	int32 Rows = 7;
	
	UFUNCTION(BlueprintPure, Category = "BFS")
	bool IsInRange(int32 Y, int32 X) const;
	

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Field")
	float TileWidthX = 220.f;

	// Field 타일 시작점 로컬 좌표
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Field")
	FVector2D FirstFieldLocal = FVector2D(-580,-520);

	// 트루시 아래부터 위로 쌓음 false의 경우 위에서 아래로 쌓임 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Field")
	bool bRowZeroAtBottom = true;

	// 지그재그로 쌓이게 만들기
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Field")
	float OddColumRowShift = -0.5f;

	// 벤치 타일 설정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bench")
	int32 BenchSize = 9;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bench")
	int32 BenchSlotsPerSide = 9;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bench")
	FVector2D BenchTilePlayerLocal = FVector2D(-760.f, -520.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bench")
	FVector2D BenchTileEnemyLocal = FVector2D(760.f, 520.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bench")
	float BenchStepLocalY = 150.f;

	// 시계방향 배치 여부
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bench")
	bool bBenchClockwise = false;

	// 에디터 원클릭 초기화
	UFUNCTION(BlueprintCallable, CallInEditor)
	void QuickSetUp();

	// 필드 배치 / 삭제 / 조회
	UFUNCTION(BlueprintCallable, Category = "Field")
	bool PlaceUnitOnField(int32 Y, int32 X, APCBaseUnitCharacter* Unit);

	UFUNCTION(BLueprintCallable, Category = "Field")
	bool RemoveFromField(int32 Y, int32 X);

	UFUNCTION(BlueprintPure, Category = "Field")
	APCBaseUnitCharacter* GetFieldUnit(int32 Y, int32 X) const;

	UFUNCTION(BlueprintPure, Category = "Field")
	FVector GetFieldUnitLocation(APCBaseUnitCharacter* Unit) const;

	UFUNCTION(BlueprintPure, Category = "Field")
	FIntPoint GetFiledUnitGridPoint(APCBaseUnitCharacter* Unit) const;

	// 월드 / 로컬 포지션 제공
	UFUNCTION(BlueprintPure, Category = "Field")
	FVector GetTileWorldPosition(int32 Y, int32 X) const;

	UFUNCTION(BlueprintPure, Category = "Field")
	FVector GetTileLocalPosition(int32 Y, int32 X) const;

	UFUNCTION(BlueprintPure, Category = "Field")
	FVector GetTilePosition(int32 Y, int32 X) const { return GetTileWorldPosition(Y, X);}

	// 벤치 배치 / 삭제 / 조회
	UFUNCTION(BlueprintCallable, Category = "Bench")
	bool PlaceUnitOnBench(int32 BenchIndex, APCBaseUnitCharacter* Unit);

	UFUNCTION(BlueprintCallable, Category = "Bench")
	bool RemoveFromBench(int32 BenchIndex);

	UFUNCTION(BlueprintCallable, Category = "Bench")
	APCBaseUnitCharacter* GetBenchUnit(int32 BenchIndex) const;

	UFUNCTION(BlueprintCallable, Category = "Bench")
	FVector GetBenchWorldPosition(int32 BenchIndex) const;

	UFUNCTION(BlueprintCallable, Category = "Bench")
	FVector GetBenchLocalPosition(int32 BenchIndex) const;

	UFUNCTION(BlueprintPure, Category = "Bench")
	int32 MirrorBenchIndex(int32 Index) const;

	UFUNCTION(BlueprintPure, Category = "Bench")
	int32 GetBenchIndex(bool bEnemySide, int32 LocalIndex) const;

	// 유틸 함수
	UFUNCTION(BlueprintCallable, Category="Util")
	APCCombatBoard* GetCombatBoard() const;
	
	UFUNCTION(BlueprintCallable, Category = "Util")
	void ClearAll();

	UFUNCTION(BlueprintCallable, Category = "Util")
	void MoveUnitsMirroredTo(UPCTileManager* TargetField, bool bMirrorRows = true, bool bMirrorCols = true, bool bIncludeBench = true );

	UPROPERTY(EditAnywhere, Category = "Data")
	TArray<FTile> Field;

	UPROPERTY(EditAnywhere, Category = "Data")
	TArray<FTile> Bench;

private:
	void CreateField(); // 필드 좌표 생성 (월드기준)
	void CreateBench(); // 벤치 좌표 생성 (월드기준)

	TWeakObjectPtr<APCCombatBoard> CachedCombatBoard;
			


	// 디버그용

public:

	// 디버그 표시용
	UPROPERTY(EditAnywhere, Category="Debug") bool bDebugShowIndices = true;
	UPROPERTY(EditAnywhere, Category="Debug") float DebugPointRadius = 18.f;   // 구 반지름
	UPROPERTY(EditAnywhere, Category="Debug") float DebugTextScale  = 1.0f;    // 라벨 스케일
	UPROPERTY(EditAnywhere, Category="Debug") float DebugDuration   = 100.f;     // 지속시간(초). 0이면 1프레임

	// 색상
	UPROPERTY(EditAnywhere, Category="Debug") FColor FieldColor   = FColor(0,255,127); // 민트
	UPROPERTY(EditAnywhere, Category="Debug") FColor BenchAColor  = FColor::Cyan;      // 내 벤치(0..N-1)
	UPROPERTY(EditAnywhere, Category="Debug") FColor BenchBColor  = FColor::Orange;    // 적 벤치(N..2N-1)
	UPROPERTY(EditAnywhere, Category="Debug") FColor UnitColor    = FColor::Yellow;    // 유닛 있는 칸

	// 에디터 버튼 + 런타임 호출 가능
	UFUNCTION(BlueprintCallable, CallInEditor, Category="Debug")
	void DebugDrawTiles(bool bPersistent = true);
	
};
