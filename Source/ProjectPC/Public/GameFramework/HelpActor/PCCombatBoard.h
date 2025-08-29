// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PCCombatBoard.generated.h"

class APCUnitVisual;
struct FGameplayTag;
class APCPlayerState;
class UHierarchicalInstancedStaticMeshComponent;
class UCameraComponent;
class USpringArmComponent;

// 타일 마커를 모아둔 정보 (월드 변환 등)
USTRUCT()
struct FTileInfo
{
	GENERATED_BODY()

	UPROPERTY()
	FTransform WorldTransform;
};

UCLASS()
class PROJECTPC_API APCCombatBoard : public AActor
{
	GENERATED_BODY()
	
public:	
	APCCombatBoard();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	USceneComponent* SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UCameraComponent* BoardCamera;

	// 해당 보드 번호 (SeatIndex와 1:1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Board")
	int32 BoardSeatIndex = 0;

	// SeatAnchor 소켓 부모를 에디터에서 지정
	UPROPERTY(EditAnywhere, Category = "Seat")
	FComponentReference PlayerSeatParent;

	UPROPERTY(EditAnywhere, Category = "Seat")
	FComponentReference EnemySeatParent;

	UPROPERTY(EditAnywhere, Category = "Seat")
	FName PlayerSeatSocket = TEXT("SeatPlayer_Socket");

	UPROPERTY(EditAnywhere, Category = "Seat")
	FName EnemySeatSocket = TEXT("SeatEnemy_Socket");

	UPROPERTY(EditAnywhere, Category = "Seat")
	FVector SeatExtraOffset = FVector(0,0,0);

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Seat")
	USceneComponent* PlayerSeatAnchor = nullptr;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Seat")
	USceneComponent* EnemySeatAnchor = nullptr;

	UFUNCTION(BlueprintCallable)
	FTransform GetPlayerSeatTransform() const;

	UFUNCTION(BlueprintCallable)
	FTransform GetEnemySeatTransform() const;

	// 로컬 플레이어 카메라
	UFUNCTION(BlueprintCallable)
	void ApplyLocalBottomView(class APlayerController* PlayerController, int32 MySeatIndex, float Blend = 0.35f);

protected:
	
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type reason) override;

#if WITH_EDITOR
	virtual void OnConstruction(const FTransform& Transform) override;
#endif

	void RebuildAnchors();
	USceneComponent* Resolve(const FComponentReference& Ref) const;


	// 타일 관련
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Markers", meta=(AllowPrivateAccess = "true"))
	USceneComponent* FieldRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Markers", meta=(AllowPrivateAccess = "true"))
	USceneComponent* BenchRoot;

	UPROPERTY(EditAnywhere, Category = "Markers")
	FName FieldPrefix = TEXT("Field_");

	UPROPERTY(EditAnywhere, Category = "Markers")
	FName BenchPrefix = TEXT("Bench_");

	// HISM(필드/벤치)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HISM", meta=(AllowPrivateAccess = "true"))
	UHierarchicalInstancedStaticMeshComponent* FieldHISM;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HISM", meta=(AllowPrivateAccess = "true"))
	UHierarchicalInstancedStaticMeshComponent* BenchHISM;

	UPROPERTY(EditAnywhere, Category = "HISM")
	UStaticMesh* HexTileMesh = nullptr;

	UPROPERTY(EditAnywhere, Category = "HISM")
	UMaterialInterface* HexTileMaterial = nullptr;

	UPROPERTY(EditAnyWHere, Category = "HISM")
	UMaterialInterface* HexTileOverlayMaterial = nullptr;

	UPROPERTY(EditAnywhere, Category = "HISM")
	UStaticMesh* BenchTileMesh = nullptr;

	UPROPERTY(EditAnywhere, Category = "HISM")
	UMaterialInterface* BenchTileMaterial = nullptr;

	UPROPERTY(EditAnywhere, Category = "HISM")
	UMaterialInterface* BenchTileOverlayMaterial = nullptr;

	// 유닛 비주얼 클래스
	UPROPERTY(EditAnywhere, Category = "UnitViz")
	TSubclassOf<APCUnitVisual> UnitVisualClass;

	// 공개 API
	UFUNCTION(BlueprintCallable)
	void RebuildTilesFromMarkers();

	// 마우스 히트 -> (x,y) / 벤치 여부로 변환
	bool TryGetTileFromHit(const FHitResult& Hit, int32& OutX, int32& OutY, bool& bBench) const;

	// PlayerState 변경되면 호출 : 비주얼 동기화
	void UpdateBoardFromPlayerState(APCPlayerState* PCPlayerState);

protected:
	// 마커 수집
	void CollectTileMarkers();

	// 인스터스 / 매핑 구축
	void BuildHISM();

	// 유닛 비주얼 풀 관리
	void SpawnOrMoveVisual(bool bBench, int32 TileIndex, FGameplayTag UnitID, const FTransform& Transform);
	void GarbageUnusedVisual(bool bBench, const TBitArray<>& Used);

	// 내부 키 (벤치/ 필드 구분 포함)
	static int32 MakeKey(bool bBench, int32 TileIndex) { return TileIndex + (bBench ? 100000 : 0);}


private:

	// 타일 월드 변환
	UPROPERTY()
	TArray<FTileInfo> FieldTiles;
	UPROPERTY()
	TArray<FTileInfo> BenchTiles;

	// HISM 인덱스 / 논리 좌표
	UPROPERTY()
	TArray<FIntPoint> Field_InstanceToXY;
	UPROPERTY()
	TArray<FIntPoint> Bench_InstanceToXY;

	// (벤치/타일) -> 비주얼 엑터
	UPROPERTY(Transient)
	TMap<int32, TWeakObjectPtr<APCUnitVisual>> Visuals;

	// 내가 구독중인 PlayerState
	UPROPERTY()
	TWeakObjectPtr<APCPlayerState> BoundPCPlayerState;
	FDelegateHandle BoundHandle;
	
	
};
