// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PCPlayerBoard.generated.h"

class UPCTileManager;
class APCCombatBoard;
class APCPlayerState;
class APCBaseUnitCharacter;
class UHierarchicalInstancedStaticMeshComponent;

USTRUCT(BlueprintType)
struct FPlayerTile
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FIntPoint UnitIntPoint = FIntPoint::NoneValue;
		
	UPROPERTY(Transient, BlueprintReadWrite)
	FVector Position = FVector::ZeroVector;
	
	UPROPERTY(BlueprintReadWrite)
	APCBaseUnitCharacter* Unit = nullptr;

	UPROPERTY(BlueprintReadWrite)
	bool bIsField = true;
	
	bool IsEmpty() const { return Unit == nullptr; }
	
};

USTRUCT(BlueprintType)
struct FPlayerBoardSnapshot
{
	GENERATED_BODY()
	UPROPERTY() TArray<APCBaseUnitCharacter*> FieldUnits; // Rows*Cols 크기
	UPROPERTY() TArray<APCBaseUnitCharacter*> BenchUnits; // BenchSize 크기
};


UCLASS()
class PROJECTPC_API APCPlayerBoard : public AActor
{
	GENERATED_BODY()
	
public:	
	
	APCPlayerBoard();

	// 월드에 배치할 때 지정하는 식별 좌석 번호
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "PlayerBoard")
	int32 PlayerIndex = -1;

	// 소유자 확인용
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "PlayerBoard")
	APCPlayerState* OwnerPlayerState = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	USceneComponent* SceneRoot = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Field")
	int32 Cols = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Field")
	int32 Rows = 7;

	FORCEINLINE int32 IndexOf(int32 Y, int32 X) const
	{
		check(Y >= 0 && Y < Cols && X >= 0 && X < Rows);
		return Y * Rows + X;
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Field")
	float FieldTileWidthX = 220.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Field")
	FVector2D FirstFieldLoc = FVector2D(-580,-520);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Field")
	bool bRowZeroAtBottom = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Field")
	float OddColumRowShift = -0.5f;

	// 벤치타일

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bench")
	int32 BenchSize = 9;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bench")
	FVector2D FirstBenchLoc = FVector2D(-760.f, -520.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bench")
	float BenchStepLocalY = 150.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bench")
	bool bBenchClockWise = true;

	UFUNCTION(BlueprintCallable, CallInEditor)
	void QuickSetUp();

	UPROPERTY(BlueprintReadOnly, Category = "FieldData")
	TArray<FPlayerTile> PlayerField;

	UPROPERTY(BlueprintReadOnly, Category = "FieldData")
	TArray<FPlayerTile> PlayerBench;

	// HISM
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HISM", meta = (AllowPrivateAccess = "true"));
	UHierarchicalInstancedStaticMeshComponent* PlayerFieldHISM;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HISM", meta = (AllowPrivateAccess = "true"));
	UHierarchicalInstancedStaticMeshComponent* PlayerBenchHISM;

	UPROPERTY(EditAnywhere, Category = "HISM")
	UStaticMesh* FieldTileMesh = nullptr;

	UPROPERTY(EditAnywhere, Category = "HISM")
	UMaterialInterface* FieldTileMaterial = nullptr;

	UPROPERTY(EditAnywhere, Category = "HISM")
	UMaterialInterface* FieldTileOverlayMaterial = nullptr;

	UPROPERTY(EditAnywhere, Category = "HISM")
	UStaticMesh* BenchTileMesh = nullptr;

	UPROPERTY(EditAnywhere, Category = "HISM")
	UMaterialInterface* BenchTileMaterial = nullptr;

	UPROPERTY(EditAnywhere, Category = "HISM")
	UMaterialInterface* BenchTileOverlayMaterial = nullptr;

	UFUNCTION(BlueprintCallable, Category = "HISM")
	void OnHISM(bool bIsOn, bool bIsBattle);
	
	void BuildHISM();

protected:
	
	virtual void BeginPlay() override;

	UPROPERTY(ReplicatedUsing=OnRep_FieldLocs)
	TArray<FVector_NetQuantize10> FieldLocs;

	UPROPERTY(ReplicatedUsing=OnRep_BenchLocs)
	TArray<FVector_NetQuantize10> BenchLocs;

	UFUNCTION()
	void OnRep_FieldLocs();

	UFUNCTION()
	void OnRep_BenchLocs();

	void RebuildHISM_FromArrays();

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

#if WITH_EDITOR
	virtual void OnConstruction(const FTransform& Transform) override;
#endif

public:
	void CreatePlayerField();
	void CreatePlayerBench();

	// ─────────────────────────────────────────────────────────────
    // 1) 쿼리/조회
    UFUNCTION(BlueprintCallable, Category="PlayerBoard|Query")
    APCBaseUnitCharacter* GetFieldUnit(int32 Y, int32 X) const;

    UFUNCTION(BlueprintCallable, Category="PlayerBoard|Query")
    APCBaseUnitCharacter* GetBenchUnit(int32 LocalBenchIndex) const;

    UFUNCTION(BlueprintCallable, Category="PlayerBoard|Query")
    int32 GetBenchUnitIndex(APCBaseUnitCharacter* Unit) const;

    UFUNCTION(BlueprintCallable, Category="PlayerBoard|Query")
    FIntPoint GetFieldUnitGridPoint(APCBaseUnitCharacter* Unit) const;

    UFUNCTION(BlueprintCallable, Category="PlayerBoard|Query")
    bool IsInRange(int32 Y, int32 X) const;

    UFUNCTION(BlueprintCallable, Category="PlayerBoard|Query")
    bool IsTileFree(int32 Y, int32 X) const;

    UFUNCTION(BlueprintCallable, Category="PlayerBoard|Query")
    int32 GetFirstEmptyBenchIndex() const;

	UFUNCTION(BlueprintCallable, Category = "LevelUp")
	TArray<APCBaseUnitCharacter*> GetAllUnitByTag(FGameplayTag UnitTag, int32 TeamSeat);

	UFUNCTION(BlueprintCallable, Category = "LevelUp")
	TArray<APCBaseUnitCharacter*> GetFieldUnitByTag(FGameplayTag UnitTag);
	
	UFUNCTION(BlueprintCallable, Category = "LevelUp")
	TArray<APCBaseUnitCharacter*> GetBenchUnitByTag(FGameplayTag UnitTag, int32 TeamSeat);

    // ─────────────────────────────────────────────────────────────
    // 2) 배치/이동 (플레이 중)
    UFUNCTION(BlueprintCallable, Category="PlayerBoard|Placement")
    bool PlaceUnitOnField(int32 Y, int32 X, APCBaseUnitCharacter* Unit);

    UFUNCTION(BlueprintCallable, Category="PlayerBoard|Placement")
    bool PlaceUnitOnBench(int32 LocalBenchIndex, APCBaseUnitCharacter* Unit, bool bIsBattle);

    UFUNCTION(BlueprintCallable, Category="PlayerBoard|Placement")
    bool RemoveFromField(int32 Y, int32 X);

    UFUNCTION(BlueprintCallable, Category="PlayerBoard|Placement")
    bool RemoveFromBench(int32 LocalBenchIndex);

    UFUNCTION(BlueprintCallable, Category="PlayerBoard|Placement")
    bool RemoveFromBoard(APCBaseUnitCharacter* Unit); // 필드/벤치 어디에 있든 제거

    UFUNCTION(BlueprintCallable, Category="PlayerBoard|Placement")
    bool Swap(APCBaseUnitCharacter* A, APCBaseUnitCharacter* B);

    // ─────────────────────────────────────────────────────────────
    // 3) 월드좌표 → 보드 타일/벤치 히트 (드래그&드랍 대체)
    UFUNCTION(BlueprintCallable, Category="PlayerBoard|HitTest")
    bool WorldToField(const FVector& World, int32& OutY, int32& OutX, float MaxSnapDist=0.f) const;

    UFUNCTION(BlueprintCallable, Category="PlayerBoard|HitTest")
    bool WorldToBench(const FVector& World, int32& OutLocalBenchIndex, float MaxSnapDist=0.f) const;

    UFUNCTION(BlueprintCallable, Category="PlayerBoard|HitTest")
    bool WorldAnyTile(const FVector& World, bool bPreferField,
                      bool& bOutIsField, int32& OutY, int32& OutX, int32& OutLocalBenchIndex,
                      FVector& OutSnapPos, float MaxSnapField=0.f, float MaxSnapBench=0.f,
                      bool bRequireUnit=false) const;
	
    // ─────────────────────────────────────────────────────────────
    // 4) 전투 연계: CombatBoard/TileManager와 동기화(필드만!)
    UFUNCTION(BlueprintCallable, Category="PlayerBoard|Battle")
    void AttachToCombatBoard(APCCombatBoard* CombatBoardToFollow, bool bIsGuest);

    UFUNCTION(BlueprintCallable, Category="PlayerBoard|Battle")
    void DetachFromCombatBoard();

	UFUNCTION(BlueprintCallable, Category="PlayerBoard|Battle")
	void ResnapBenchUnitsToBoard(bool bIsBattle); // 전투 종료시 호출 (벤치 복귀)
	
	// 평시 위치 / 회전 복구용
	UPROPERTY(Transient)
	FTransform SaveHomeTransform;

	// 전투시 붙히는 보드
	UPROPERTY(Transient)
	TWeakObjectPtr<APCCombatBoard> BattleBoard;

	UPROPERTY(EditAnywhere, Category="PlayerBoard|Battle")
	float AttachYawOffsetDeg = 180.f;

	UPROPERTY(EditAnywhere, Category="PlayerBoard|Battle")
	FVector AttachWorldOffset = FVector::ZeroVector;

    // 플레이 보드(Field) → 전투 TM(Field) 복사 (전투 시작 시)
    UFUNCTION(BlueprintCallable, Category="PlayerBoard|Battle")
    bool CopyFieldToTileManager(UPCTileManager* TM, bool bMirrorRows, bool bMirrorCols);

    // 전투 TM(Field) → 플레이 보드(Field) 복사 (필요 시, 전투 종료 후 복원용)
    UFUNCTION(BlueprintCallable, Category="PlayerBoard|Battle")
    bool CopyTileManagerToField(UPCTileManager* TM, bool bMirrorRows, bool bMirrorCols);

    // ─────────────────────────────────────────────────────────────
    // 5) 하이라이트/HISM 표시 제어 (CombatBoard의 HISM 제거 대비)
    UFUNCTION(BlueprintCallable, Category="PlayerBoard|Visual")
    void SetHighlight(bool bOnField, bool bOnBench);

    // ─────────────────────────────────────────────────────────────
    // 6) 스냅샷/직렬화 (선택: 디버깅/저장/복원)

    UFUNCTION(BlueprintCallable, Category="PlayerBoard|Snapshot")
    void MakeSnapshot(FPlayerBoardSnapshot& Out) const;

    UFUNCTION(BlueprintCallable, Category="PlayerBoard|Snapshot")
    void ApplySnapshot(const FPlayerBoardSnapshot& In);
	
    // 내부 헬퍼
    bool EnsureExclusive(APCBaseUnitCharacter* Unit);
    FVector GetFieldWorldPos(int32 Y, int32 X);
    FVector GetBenchWorldPos(int32 LocalBenchIndex);

	static FORCEINLINE FVector ToWorld(const USceneComponent* Root, const FVector& Local)
	{
		return Root ? Root->GetComponentTransform().TransformPosition(Local) : Local;
	}
	static FORCEINLINE FVector ToLocal(const USceneComponent* Root, const FVector& World)
	{
		return Root ? Root->GetComponentTransform().InverseTransformPosition(World) : World;
	}
};
	

