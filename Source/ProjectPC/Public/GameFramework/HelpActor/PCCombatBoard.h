// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/HelpActor/PCTileType.h"
#include "PCCombatBoard.generated.h"

class UAbilitySystemComponent;
struct FOnAttributeChangeData;
class UPCGoldDisplayComponent;
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

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	USceneComponent* SceneRoot;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UCameraComponent* BoardCamera;

	// 해당 보드 번호 (SeatIndex와 1:1)
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Board")
	int32 BoardSeatIndex = -1;

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

	FVector HomeCameraOffset = FVector(-110.f, 0.f, -110.f);
	FRotator HomeCameraRotation = FRotator(3.f, 0.f,0.f);
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
	FVector HomeCam_LocPreset = FVector(-1150.f,0.f, 1000.f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
	FRotator HomeCam_RocPreset = FRotator(-50.f, 0.f, 0.f);
	
	UPROPERTY(EditAnywhere, Category= "Camera")
	FVector BattleCameraChangeLocation = FVector(1150.f, 0.f, 1000.f);

	UPROPERTY(EditAnywhere, Category= "Camera")
	FRotator BattleCameraChangeRotation = FRotator(-50.f, 180.f,0.f);

	// GoldDisplayComponent
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GoldDisplay")
	TObjectPtr<UPCGoldDisplayComponent> MyGoldDisplay;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GoldDisplay")
	TObjectPtr<UPCGoldDisplayComponent> EnemyGoldDisplay;
	
	// 멀티케스트에서 호출될 표시 함수
	void ApplyMyGoldVisual(int32 NewGold);
	void ApplyEnemyGoldVisual(int32 NewGold);
	
private:
		
	APCPlayerState* FindPSBySeat(int32 SeatIndex) const;
	

protected:
	
	virtual void BeginPlay() override;
	virtual void PostInitializeComponents() override;

#if WITH_EDITOR
	virtual void OnConstruction(const FTransform& Transform) override;
#endif

	void RebuildAnchors();
	USceneComponent* Resolve(const FComponentReference& Ref) const;

	
	// Tile Manager
public:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = true), Category = "Tile")
	UPCTileManager* TileManager;

	// 보드에서 바로 타일 쿼리하고 싶을때 (래퍼)
	UFUNCTION(BlueprintPure, Category = "Tile")
	APCBaseUnitCharacter* GetUnitAt(int32 Y, int32 X) const;

	// 광역 궁극기 구현을 위한 헬퍼 함수 // WDH
	UFUNCTION(Category="Tile")
	void GetAllFieldUnits(TArray<TWeakObjectPtr<APCBaseUnitCharacter>>& FieldUnits) const;
	
	UFUNCTION(BlueprintPure, Category = "Tile")
	FVector GetFieldUnitLocation(APCBaseUnitCharacter* InUnit) const;

	UFUNCTION(BlueprintPure, Category = "Tile")
	FIntPoint GetFieldUnitPoint(APCBaseUnitCharacter* InUnit) const;

	UFUNCTION(BlueprintPure, Category = "Tile")
	FVector GetTileWorldLocation(int32 Y, int32 X) const;
	
	UFUNCTION(BlueprintPure, Category = "Tile")
	bool IsInRange(int32 Y, int X) const;

	// BFS 점유 예약 관련
	// 완전히 비어 있는가? (점유, 예약 둘다 x)
	UFUNCTION(BlueprintPure, Category = "Tile")
	bool IsTileFree(int32 Y, int32 X) const;

	// 이 유닛이 지금 이동 할 수 있는가? ( 비었거나/ 내 점유 / 내 예약 )
	UFUNCTION(BlueprintPure, Category = "Tile")
	bool CanUse(int32 Y, int32 X, const APCBaseUnitCharacter* InUnit);

	// 해당 유닛이 어떤 타일이든 예약을 하나라도 가지고 있는가?
	UFUNCTION(BlueprintPure, Category = "Tile")
	bool HasAnyReservation(const APCBaseUnitCharacter* InUnit);

	// 예약/ 점유/ 해제 : 하나의 함수로 상태 전이 (성공 여부 반환)
	UFUNCTION(BlueprintCallable, Category = "Tile")
	bool SetTileState(int32 Y, int32 X, APCBaseUnitCharacter* InUnit, ETileAction Action);
	
	// 그 유닛이 갖고있던 타일 상태 전부 풀기 ( 사망 / 취소 시)
	UFUNCTION(BlueprintCallable, Category = "Tile")
	void ClearAllForUnit(APCBaseUnitCharacter* InUnit);
	
};
