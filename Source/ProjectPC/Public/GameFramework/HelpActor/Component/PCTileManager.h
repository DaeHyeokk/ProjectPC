// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/HelpActor/PCTileType.h"
#include "PCTileManager.generated.h"

struct FGameplayTag;

class APCCombatBoard;
class APCBaseUnitCharacter;

USTRUCT(BlueprintType)
struct FTile
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FIntPoint UnitIntPoint = FIntPoint::NoneValue;
		
	UPROPERTY(BlueprintReadWrite)
	FVector Position = FVector::ZeroVector;
	
	UPROPERTY(BlueprintReadWrite)
	APCBaseUnitCharacter* Unit = nullptr;

	UPROPERTY(BlueprintReadWrite)
	bool bIsField = true;

	UPROPERTY()
	TWeakObjectPtr<APCBaseUnitCharacter> ReservedUnit;

	bool IsEmpty() const { return Unit == nullptr; }
	bool IsReserved() const { return ReservedUnit.IsValid(); }
	bool IsFree() const { return !Unit && !ReservedUnit.IsValid(); }
	bool IsOwnedBy(const APCBaseUnitCharacter* TestUnit) const { return Unit == TestUnit; }
	bool IsReservedBy(const APCBaseUnitCharacter* TestUnit) const { return ReservedUnit.Get() == TestUnit; }
	bool CanBeUsedBy(const APCBaseUnitCharacter* TestUnit) const
	{
		return IsFree() || IsOwnedBy(TestUnit) || IsReservedBy(TestUnit);
	}
};

UENUM()
enum class ETileFacing : uint8 { Auto, Friendly, Enemy };


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTPC_API UPCTileManager : public UActorComponent
{
	GENERATED_BODY()

public:	
	
	UPCTileManager();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seat")
	int32 BoardIndex;
	
	// 필드 타일 크기 ( 8 * 7 )
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Field")
	int32 Cols = 8;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Field")
	int32 Rows = 7;
	
	UFUNCTION(BlueprintPure, Category = "BFS")
	bool IsInRange(int32 Y, int32 X) const;

	// 헷갈림 방지 고정 
	FORCEINLINE int32 IndexOf(int32 Y, int32 X) const { return Y * Rows + X; }

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
	bool bBenchClockwise = true;

	// 에디터 원클릭 초기화
	UFUNCTION(BlueprintCallable, CallInEditor)
	void QuickSetUp();

	// 유닛 배치시 회전값
	UPROPERTY(EditAnywhere, Category="Rotation")
	float FacingYawOffsetDeg = 0.f;

	FRotator CalcUnitRotation(APCBaseUnitCharacter* Unit, ETileFacing Facing) const;

	// 필드 배치 / 삭제 / 조회
	UFUNCTION(BlueprintCallable, Category = "Field")
	bool PlaceUnitOnField(int32 Y, int32 X, APCBaseUnitCharacter* Unit, ETileFacing FacingOverride = ETileFacing::Auto);
	
	UFUNCTION(BLueprintCallable, Category = "Field")
	bool RemoveFromField(int32 Y, int32 X, bool bPreserveUnitBoard);

	UFUNCTION(BlueprintPure, Category = "Field")
	APCBaseUnitCharacter* GetFieldUnit(int32 Y, int32 X) const;

	// 광역 궁극기 구현을 위한 헬퍼 함수 // WDH
	UFUNCTION(Category="Field")
	void GetAllFieldUnits(TArray<TWeakObjectPtr<APCBaseUnitCharacter>>& FieldUnits) const;
	
	UFUNCTION(BlueprintPure, Category = "Field")
	FVector GetFieldUnitLocation(APCBaseUnitCharacter* Unit) const;

	UFUNCTION(BlueprintPure, Category = "Field")
	FIntPoint GetFieldUnitGridPoint(APCBaseUnitCharacter* Unit) const;
	
	// 월드 / 로컬 포지션 제공
	UFUNCTION(BlueprintPure, Category = "Field")
	FVector GetTileWorldPosition(int32 Y, int32 X) const;

	UFUNCTION(BlueprintPure, Category = "Field")
	FVector GetTileLocalPosition(int32 Y, int32 X) const;

	UFUNCTION(BlueprintPure, Category = "Field")
	FVector GetTilePosition(int32 Y, int32 X) const { return GetTileWorldPosition(Y, X);}
	
	UFUNCTION(BlueprintPure, Category = "Bench")
	int32 GetBenchIndex(bool bEnemySide, int32 LocalIndex) const;

	// 필드, 벤치 통합 제거
	UFUNCTION(BlueprintCallable, Category = "Field/Bench")
	bool RemoveFromBoard(APCBaseUnitCharacter* Unit);
	
	// 점유 관련 헬퍼

	// 완전히 비어있는가? (점유, 예약 둘다 X)
	UFUNCTION(BlueprintPure, Category = "BFS")
	bool IsTileFree(int32 Y, int32 X) const;

	// 이 유닛이 지금 사용(이동) 할수 있는가?
	UFUNCTION(BlueprintPure, Category = "BFS")
	bool CanUse(int32 Y, int32 X, const APCBaseUnitCharacter* InUnit) const;
	
	// 해당 유닛이 어떤 타일이든 예약을 하고 있는가?
	UFUNCTION(BlueprintPure, Category = "BFS")
	bool HasAnyReservation(const APCBaseUnitCharacter* InUnit) const;

	// 타일 예약/점유/해제 상태 설정 함수
	UFUNCTION(BlueprintCallable, Category = "BFS")
	bool SetTileState(int32 Y, int32 X, APCBaseUnitCharacter* InUnit, ETileAction Action);

	// 그 유닛이 가지고 있던 점유 예약 전부 해제(사망, 취소)
	UFUNCTION(BlueprintCallable, Category = "BFS")
	void ClearAllForUnit(APCBaseUnitCharacter* InUnit);
	

	// 유틸 함수
	UFUNCTION(BlueprintCallable, category = "Util")
	APCCombatBoard* GetCombatBoard() const;

	UFUNCTION(BlueprintCallable, category = "Util")
	int32 GetBoardIndex();
	
	UFUNCTION(BlueprintCallable, Category = "Util")
	void ClearAll();
	
	bool EnsureExclusive(APCBaseUnitCharacter* InUnit);
	
	UPROPERTY(BlueprintReadOnly, Category = "Data")
	TArray<FTile> Field;

	// 전투 승리 유닛 팀인덱스로 찾기
	TArray<APCBaseUnitCharacter*> GetWinnerUnitByTeamIndex(int32 WinnerTeamIndex);

	// 살아남은 모든 유닛 가져오기
	TArray<APCBaseUnitCharacter*> GetAllAliveUnit();

private:
	void CreateField(); // 필드 좌표 생성 (월드기준)

	UPROPERTY()
	TObjectPtr<APCCombatBoard> CachedCombatBoard;

	virtual void BeginPlay() override;
	
	bool IsValidTile(int32 Y, int32 X, int32& OutIndex) const;
	
	// 디버그용

public:

	// 디버그 표시용
	UPROPERTY(EditAnywhere, Category="Debug") bool bDebugShowIndices = true;
	UPROPERTY(EditAnywhere, Category="Debug") float DebugPointRadius = 18.f;   // 구 반지름
	UPROPERTY(EditAnywhere, Category="Debug") float DebugTextScale  = 1.0f;    // 라벨 스케일
	UPROPERTY(EditAnywhere, Category="Debug") float DebugDuration   = 10.f;     // 지속시간(초). 0이면 1프레임

	// 색상
	UPROPERTY(EditAnywhere, Category="Debug") FColor FieldColor   = FColor(0,255,127); // 민트
	UPROPERTY(EditAnywhere, Category="Debug") FColor BenchAColor  = FColor::Cyan;      // 내 벤치(0..N-1)
	UPROPERTY(EditAnywhere, Category="Debug") FColor BenchBColor  = FColor::Orange;    // 적 벤치(N..2N-1)
	UPROPERTY(EditAnywhere, Category="Debug") FColor UnitColor    = FColor::Yellow;    // 유닛 있는 칸

	// 에디터 버튼 + 런타임 호출 가능
	UFUNCTION()
	void DebugLogField(bool bAsGrid /*=true*/, bool bShowOccupiedList /*=true*/, const FString& Tag) const;

	UFUNCTION(BlueprintCallable, Category="Debug")
	void DebugExplainTile(int32 Y, int32 X, const FString& Tag) const;


	// === Debug helpers ===
	UFUNCTION(BlueprintCallable, Category="Debug")
	void DebugDrawTiles(float Duration = 10.f, bool bPersistent = true, bool bShowIndex = true, bool bShowYX = true, bool bShowUnit = true) const;

	UFUNCTION(BlueprintCallable, Category="Debug")
	void DebugClearPersistent() const;

#if WITH_EDITOR
	// 디테일 패널에서 바로 실행(에디터/비PIE)
	UFUNCTION(CallInEditor, Category="Debug")
	void Editor_DrawTilesPersistent();

	UFUNCTION(CallInEditor, Category="Debug")
	void Editor_ClearDebug();
#endif

private:
	FString DescribeTileState(int32 Index);
	

#pragma region Win&Lose

public:
	void BindToUnit(APCBaseUnitCharacter* Unit);
	void UnbindFromUnit(APCBaseUnitCharacter* Unit);

	UFUNCTION()
	void OnBoundUnitDied(APCBaseUnitCharacter* Unit);

protected:
	UPROPERTY()
	TSet<TWeakObjectPtr<APCBaseUnitCharacter>> DeathBoundUnits;
#pragma endregion
};
