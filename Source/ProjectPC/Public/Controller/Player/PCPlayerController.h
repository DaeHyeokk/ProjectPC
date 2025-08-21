// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PCPlayerController.generated.h"

class APCPlayerCameraRig;
/**
 * 
 */
UCLASS()
class PROJECTPC_API APCPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	UFUNCTION(Client, Reliable)
	void Client_FocusView(const FTransform& View, float BlendTime);

	// 추후 플레이어에서 구현
	// void OnZoomIn();
	// void OnZoomOut();

private:
	UPROPERTY()
	APCPlayerCameraRig* PlayerCameraRig = nullptr;

	void EnsureCameraRig();
	
};
