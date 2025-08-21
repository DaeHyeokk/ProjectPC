// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PCCarouseRing.generated.h"

class URotatingMovementComponent;

UCLASS()
class PROJECTPC_API APCCarouseRing : public AActor
{
	GENERATED_BODY()
	
public:	
	
	APCCarouseRing();

	// 슬롯 / 지오메트리
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ring")
	int32 NumSlots = 8;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ring")
	float Radius = 900.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ring")
	float Height = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ring")
	float StartAngleDeg = 90.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ring")
	bool bFaceCenter = true;

	// 회전
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rotate")
	bool bRotate = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rotate")
	float RotationRateYawDeg = 35.f;

	// 회전초밥에 올릴 기물 설정

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PickUp")
	TSubclassOf<AActor> PickupClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PickUp")
	int32 NumPickups = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PickUp")
	FVector PickupOffset = FVector(0,0,30);

	// 스폰, 정리, 회전제어

	UFUNCTION(BlueprintCallable, Category = "Carousel")
	void SpawnPickUps();

	UFUNCTION(BlueprintCallable, Category = "Carousel")
	void ClearPickUps();

	UFUNCTION(BlueprintCallable, Category = "Carousel")
	void SetRotationActive(bool bOn);

	// 슬롯 변환(월드)
	UFUNCTION(BlueprintCallable, Category = "Ring")
	FTransform GetSlotTransformWorld(int32 Index) const;

protected:

	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere)
	URotatingMovementComponent* RotatingMovement = nullptr;

	UPROPERTY(Transient)
	TArray<TWeakObjectPtr<AActor>> SpawnedPickupActors;

#if WITH_EDITOR
public:
	virtual void OnConstruction(const FTransform& transform) override;
	
	bool bDrawDebug = true;
	FColor DebugColor = FColor::Green;

	
#endif
	
	
};
