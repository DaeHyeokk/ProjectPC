// Fill out your copyright notice in the Description page of Project Settings.


#include "Controller/Player/PCPlayerController.h"

#include "Character/Player/Camera/PCPlayerCameraRig.h"


void APCPlayerController::BeginPlay()
{
	Super::BeginPlay();
	bShowMouseCursor = true;
	FInputModeGameAndUI InputGameMode;
	InputGameMode.SetHideCursorDuringCapture(false);
	SetInputMode(InputGameMode);

	EnsureCameraRig();

	if (PlayerCameraRig)
		SetViewTargetWithBlend(PlayerCameraRig, 0.2f);
}

void APCPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	
}

void APCPlayerController::EnsureCameraRig()
{
	if (PlayerCameraRig && IsValid(PlayerCameraRig))
		return;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = GetPawn();
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	const FVector BaseLocation = GetPawn() ? GetPawn()->GetActorLocation() : FVector::ZeroVector;
	const FRotator BaseRotation(-55.f,0.0f,0.0f);

	PlayerCameraRig = GetWorld()->SpawnActor<APCPlayerCameraRig>(APCPlayerCameraRig::StaticClass(),
		FTransform(BaseRotation, BaseLocation + FVector(0,0,500)));
}


void APCPlayerController::Client_FocusView_Implementation(const FTransform& View, float BlendTime)
{
	EnsureCameraRig();
	if (PlayerCameraRig)
	{
		PlayerCameraRig->LerpToView(View, BlendTime);
		SetViewTargetWithBlend(PlayerCameraRig, 0.15f);
	}
}


