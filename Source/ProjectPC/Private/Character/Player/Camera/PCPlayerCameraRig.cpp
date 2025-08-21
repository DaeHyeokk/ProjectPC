// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Player/Camera/PCPlayerCameraRig.h"

#include "Camera/CameraComponent.h"


// Sets default values
APCPlayerCameraRig::APCPlayerCameraRig()
{
 	PrimaryActorTick.bCanEverTick = true;

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(Root);
	SpringArm->TargetArmLength = 1200.f;
	SpringArm->bDoCollisionTest = false;
	SpringArm->bEnableCameraLag = true;
	SpringArm->SetRelativeRotation(FRotator(-55.f,0.f,0.f));

	PlayerCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("PlayerCamera"));
	PlayerCamera->SetupAttachment(SpringArm);
}


void APCPlayerCameraRig::LerpToView(const FTransform& TargetView, float Duration)
{
	StartView = GetActorTransform();
	EndView = TargetView;
	LerpTime = 0.0f;
	LerpDuration = FMath::Max(0.01f, Duration);
	bLerping = true;
}

void APCPlayerCameraRig::AddZoom(float Delta)
{
	const float NewLen = FMath::Clamp(SpringArm->TargetArmLength + Delta * ZoomStep, MinArm, MaxArm);
	SpringArm->TargetArmLength = NewLen;
}

void APCPlayerCameraRig::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!bLerping)
		return;

	LerpTime += DeltaTime;
	const float Average = FMath::Clamp(LerpTime / LerpDuration, 0.0f, 1.0f);

	const FVector Location = FMath::Lerp(StartView.GetLocation(), EndView.GetLocation(), Average);
	const FQuat Rotation = FQuat::Slerp(StartView.GetRotation(), EndView.GetRotation(), Average);
	SetActorLocationAndRotation(Location, Rotation, false);

	if (Average >= 1.0f)
		bLerping = false;

}

