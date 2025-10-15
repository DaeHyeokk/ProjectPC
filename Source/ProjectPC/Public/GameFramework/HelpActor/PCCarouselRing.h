// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PCCarouselRing.generated.h"

class APCPlayerState;
class UBoxComponent;
class APCBaseUnitCharacter;
class UCameraComponent;
class USpringArmComponent;
class URotatingMovementComponent;

UCLASS()
class PROJECTPC_API APCCarouselRing : public AActor
{
	GENERATED_BODY()
	
public:	
	
	APCCarouselRing();

	// 바깥쪽 플레이어 링
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayerRing")
	int32 PlayerNumSlots = 8;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayerRing")
	float PlayerRingRadius = 1000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayerRing")
	float PlayerRingHeight = 30.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayerRing")
	float PlayerRingStartAngleDeg = 180.f;

	// true면 플레이스 로테이션 항상 중심을 바라봄
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayerRing")
	bool bPlayerRingFaceCenter = true;

	// 안쪽 유닛 회전 링
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UnitRing")
	int32 UnitRingNumSlots = 9;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UnitRing")
	float UnitRingRadius = 400.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UnitRing")
	float UnitRingHeight = 80.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UnitRing")
	float UnitRingStartAngleDeg = 90.f;

	// 회전 on/off
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UnitRing|Rotate")
	bool bUnitRingRotate = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UnitRing|Rotate")
	float UnitRingRotationRateYawDeg = 35.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UnitRing|Pickup")
	TSubclassOf<APCBaseUnitCharacter> PickupUnit;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UnitRing|Pickup")
	int32 NumPickupsToSpawn = 9;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UnitRing|Pickup")
	FVector PickupLocalOffset = FVector(0,0,30);

	// 게이트에 사용할 스태틱 매쉬
	UPROPERTY(EditDefaultsOnly, Category = "Carousel|Gate")
	UStaticMesh* GateStaticMesh = nullptr;

	// 게이트 충돌 프리셋
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gate")
	FName GateCollisionProfile = TEXT("BlockAll");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gate", meta = (ClampMin = "5.0", ClampMax = "90.0"))
	float GateArcDeg = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gate")
	float GateThickness = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gate")
	float GateHeight = 20.f;

	UFUNCTION(BlueprintCallable, Category = "Gate")
	void BuildGates();

	UFUNCTION(BlueprintCallable, Category = "Gate")
	void OpenGateForSeat(int32 SeatIndex, bool bOpen = true);

	UFUNCTION(BlueprintCallable, Category = "Gate")
	void OpenAllGates(bool bOpen);

	// 카메라
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	bool bCameraInheritActorRotation = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float CameraArmLength = 4000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	FVector CameraArmLocalLocation = FVector(0,0,0);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	FRotator CameraArmLocalRotation = FRotator(-70,0,0);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float CameraFov = 50.f;

	UPROPERTY(EditAnywhere, Category = "Camera")
	FVector CameraPivotOffset = FVector(0.f,0.f,0.f);

	UPROPERTY(EditAnywhere, Category = "Camera")
	float CameraPitchDeg = -60.f;

	UPROPERTY(EditAnywhere, Category = "Camera")
	float CamYawBiasDeg = 0.f;

	UPROPERTY(EditAnywhere, Category = "Camera")
	float FramingPushUpZ = 200.f;

	UFUNCTION(BlueprintCallable, Category = "Camera")
	void ApplyCentralViewForSeat(APlayerController* PC, int32 SeatIndex, float BlendTime = 0.0f, float ExtraYawDeg = 0.f );


	// 슬롯 변환 제공
	UFUNCTION(BlueprintCallable, Category = "Ring")
	FTransform GetPlayerSlotTransformWorld(int32 Index) const;

	UFUNCTION(BlueprintCallable, Category = "Ring")
	FTransform GetUnitSlotTransformWorld(int32 Index) const;

	UFUNCTION(BlueprintPure, Category = "Ring")
	float GetPlayerSeatAngelDeg(int32 SeatIndex) const;

	// 픽업 제어
	UFUNCTION(BlueprintCallable, Category = "UnitRing|PickUp")
	void SpawnPickups();

	UFUNCTION(BlueprintCallable, Category = "UnitRing|PickUp")
	void ClearPickups();

	UFUNCTION(BlueprintCallable, Category = "UnitRing|PickUp")
	void SetRotationOnActive(bool bOn);


protected:

	virtual void BeginPlay() override;
	
#if WITH_EDITOR
	virtual void OnConstruction(const FTransform& transform) override;

#endif

	// 루트 / 서브 루트들
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Comp")
	USceneComponent* SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Comp")
	USceneComponent* PlayerRingRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Comp")
	USceneComponent* UnitRingRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Comp")
	USceneComponent* GateRoot;

	UPROPERTY(Transient)
	TArray<UStaticMeshComponent*> GateMeshes;
	
	// 카메라 구성요소
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	USpringArmComponent* SpringArm = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* CarouselCamera = nullptr;

	// 유닛 전용 회전 모션
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Comp")
	URotatingMovementComponent* RotatingMovement = nullptr;

	// 스폰한 픽업 게이트 핸들
	UPROPERTY(Transient)
	TArray<TWeakObjectPtr<APCBaseUnitCharacter>> SpawnedPickups;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UBoxComponent>> GateBoxes;

	// 내부 헬퍼
	FVector GetRingCenterWorld(const USceneComponent* Root) const;
	FRotator MakeFacingRotToCenter(const FVector& Pos, float ExtraYaw = 0.f) const;

	/** 디버그 */
	UPROPERTY(EditAnywhere, Category="Debug")
	bool bDrawDebug = true;

	UPROPERTY(EditAnywhere, Category="Debug")
	FColor DebugColorOuter = FColor::Cyan;

	UPROPERTY(EditAnywhere, Category="Debug")
	FColor DebugColorInner = FColor::Yellow;

	UPROPERTY(EditAnywhere, Category="Debug")
	FColor DebugColorGate  = FColor::Green;
	

	
};
