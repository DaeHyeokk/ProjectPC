// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PCCombatBoard.generated.h"

class APCBaseUnitCharacter;
class APCHeroUnitCharacter;
class UPCTileManager;
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

	// 보드 좌석 스폰용 헬퍼
	UFUNCTION(BlueprintCallable)
	FTransform GetPlayerSeatTransform() const;

	UFUNCTION(BlueprintCallable)
	FTransform GetEnemySeatTransform() const;

	// 전투용 카메라 세팅 함수

	UFUNCTION(BlueprintCallable)
	void ApplyClientHomeView();

	UFUNCTION(BlueprintCallable)
	void ApplyClientMirrorView();
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
	FVector HomeCam_LocPreset = FVector(-1150.f,0.f, 1150.f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
	FRotator HomeCam_RocPreset = FRotator(0.f, -50.f, 0.f);
	
	UPROPERTY(EditAnywhere, Category= "Camera")
	FVector BattleCameraChangeLocation = FVector(1150.f, 0.f, 1150.f);

	UPROPERTY(EditAnywhere, Category= "Camera")
	FRotator BattleCameraChangeRotation = FRotator(-50.f, 0.f,180.f);

	

protected:
	
	virtual void BeginPlay() override;

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

	// 공개 API
	UFUNCTION(BlueprintCallable)
	void RebuildTilesFromMarkers();


protected:
	// 마커 수집
	void CollectTileMarkers();

	// 인스터스 / 매핑 구축
	void BuildHISM();

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

	// Tile Manager
public:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tile")
	UPCTileManager* TileManager;

	// 보드에서 바로 타일 쿼리하고 싶을때 (래퍼)
	UFUNCTION(BlueprintPure, Category = "Tile")
	APCBaseUnitCharacter* GetUnitAt(int32 Y, int32 X) const;

	UFUNCTION(BlueprintPure, Category = "Tile")
	FVector GetFieldUnitLocation(APCBaseUnitCharacter* InUnit) const;

	UFUNCTION(BlueprintPure, Category = "Tile")
	FVector GetTileWorldLocation(int32 Y, int32 X) const;

	UFUNCTION(BlueprintPure, Category = "Tile")
	APCBaseUnitCharacter* GetBenchUnitAt(int32 BenchIndex) const;

	UFUNCTION(BlueprintPure, Category = "Tile")
	FVector GetBenchWorldLocation(int32 BenchIndex) const;

	UFUNCTION(BlueprintPure, Category = "Tile")
	bool IsInRange(int32 Y, int X) const;
	
	
};
