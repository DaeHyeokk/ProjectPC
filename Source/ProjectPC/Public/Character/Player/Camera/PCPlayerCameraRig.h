// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/SpringArmComponent.h"
#include "PCPlayerCameraRig.generated.h"

class USpringArmComponent;
class UCameraComponent;

UCLASS()
class PROJECTPC_API APCPlayerCameraRig : public AActor
{
	GENERATED_BODY()
	
public:	
	
	APCPlayerCameraRig();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UCameraComponent* PlayerCamera;

	// 카메라를 특정 TransForm으로 이동
	UFUNCTION(BlueprintCallable)
	void LerpToView(const FTransform& TargetView, float Duration = 1.0f);

	UFUNCTION(BLueprintCallable)
	void AddZoom(float Delta);

protected:
	virtual void Tick(float DeltaTime) override;

private:
	bool bLerping = false;
	float LerpTime = 0.f;
	float LerpDuration = 1.0f;
	FTransform StartView;
	FTransform EndView;

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinArm = 600.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxArm = 2200.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ZoomStep = 150.f;

	
	
};
