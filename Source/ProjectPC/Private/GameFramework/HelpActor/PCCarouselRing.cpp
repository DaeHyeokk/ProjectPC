// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFramework/HelpActor/PCCarouselRing.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/RotatingMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/KismetMathLibrary.h"


// Sets default values
APCCarouselRing::APCCarouselRing()
{
 	
	PrimaryActorTick.bCanEverTick = false;

	// 루트
	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	RotatingMovement = CreateDefaultSubobject<URotatingMovementComponent>(TEXT("RotatingMovement"));
	RotatingMovement->RotationRate = FRotator(0.f, RotationRateYawDeg, 0.f);
	RotatingMovement->bRotationInLocalSpace = false;

	// 카메라
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(Root);
	SpringArm->bDoCollisionTest = false;
	SpringArm->TargetArmLength = CameraArmLength;
	SpringArm->SetRelativeLocation(CameraArmLocalLocation);
	SpringArm->SetRelativeRotation(CameraArmLocalRotation);

	SpringArm->bInheritPitch = bCameraInheritActorRotation;
	SpringArm->bInheritYaw = bCameraInheritActorRotation;
	SpringArm->bInheritRoll = bCameraInheritActorRotation;

	CarouselCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("CarouselCamera"));
	CarouselCamera->SetupAttachment(SpringArm);
	CarouselCamera->FieldOfView = CameraFov;

	bReplicates = true;
	SetReplicateMovement(true);

}

void APCCarouselRing::BeginPlay()
{
	Super::BeginPlay();
	RotatingMovement->RotationRate.Yaw = RotationRateYawDeg;
	RotatingMovement->SetActive(bRotate);
	
	// 카메라 파라미터 반영(에디터에서 바꾼 값 유지)
	SpringArm->TargetArmLength = CameraArmLength;
	SpringArm->SetRelativeLocation(CameraArmLocalLocation);
	SpringArm->SetRelativeRotation(CameraArmLocalRotation);
	SpringArm->bInheritPitch = bCameraInheritActorRotation;
	SpringArm->bInheritYaw   = bCameraInheritActorRotation;
	SpringArm->bInheritRoll  = bCameraInheritActorRotation;
	CarouselCamera->FieldOfView = CameraFov;
}


void APCCarouselRing::SetRotationActive(bool bOn)
{
	bRotate = bOn;
	if (RotatingMovement)
		RotatingMovement->SetActive(bRotate);
}

void APCCarouselRing::SpawnPickUps()
{
	if (!HasAuthority() || !PickupClass)
		return;
	ClearPickUps();

	const int32 Count = FMath::Clamp(NumPickups, 1, NumSlots > 0 ? NumSlots : NumPickups);
	for (int32 i = 0; i < Count; ++i)
	{
		FTransform PlayerTransform = GetSlotTransformWorld(i);
		PlayerTransform.AddToTranslation(PickupOffset);
				
	}
}

void APCCarouselRing::ClearPickUps()
{
	if (!HasAuthority())
		return;
	for (auto& Weak : SpawnedPickupActors)
	{
		if (AActor* Actor = Weak.Get())
		{
			Actor->Destroy();
		}
	}
	SpawnedPickupActors.Reset();
}


FTransform APCCarouselRing::GetSlotTransformWorld(int32 Index) const
{
	const int32 SlotNum = FMath::Max(1, NumSlots);
	const float Step = 360.f / SlotNum;
	const float Deg = StartAngleDeg + Step * (Index % SlotNum);
	const float Rad = FMath::DegreesToRadians(Deg);

	const FVector Local(FMath::Cos(Rad)*Radius, FMath::Sin(Rad)* Radius, Height);
	const FVector WorldPosition = GetActorTransform().TransformPosition(Local) + FVector(0,0,20.f);

	FRotator Rotation = GetActorRotation();
	if (bFaceCenter)
	{
		Rotation = (GetActorLocation() - WorldPosition).Rotation();
	}
	return FTransform(Rotation, WorldPosition, FVector::OneVector);
}

void APCCarouselRing::ApplyCentralView(APlayerController* PlayerController, float BlendTime)
{
	if (!PlayerController)
		return;
	PlayerController->SetViewTargetWithBlend(this, BlendTime);
}


#if WITH_EDITOR
void APCCarouselRing::OnConstruction(const FTransform& transform)
{
	if (!bDrawDebug)
		return;

	UWorld* World = GetWorld();
	if (!World)
		return;

	DrawDebugCircle(World, GetActorLocation(), Radius, 64, DebugColor, false, 0.f,0,2.f,
		GetActorRightVector(), GetActorForwardVector());

	for (int32 i = 0; i < NumSlots; ++i)
	{
		const FTransform T = GetSlotTransformWorld(i);
		DrawDebugSphere(World, T.GetLocation(), 14.f, 12, DebugColor, false, 0.f,0,2.f);
	}
}

#endif
