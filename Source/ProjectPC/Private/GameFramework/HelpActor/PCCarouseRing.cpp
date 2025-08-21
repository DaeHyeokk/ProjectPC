// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFramework/HelpActor/PCCarouseRing.h"

#include "GameFramework/RotatingMovementComponent.h"


// Sets default values
APCCarouseRing::APCCarouseRing()
{
 	
	PrimaryActorTick.bCanEverTick = false;

	RotatingMovement = CreateDefaultSubobject<URotatingMovementComponent>(TEXT("RotatingMovement"));
	RotatingMovement->RotationRate = FRotator(0.f, RotationRateYawDeg, 0.f);
	RotatingMovement->bRotationInLocalSpace = false;

	bReplicates = true;
	SetReplicateMovement(true);

}

void APCCarouseRing::BeginPlay()
{
	Super::BeginPlay();
	RotatingMovement->RotationRate.Yaw = RotationRateYawDeg;
	RotatingMovement->SetActive(bRotate);

	if (HasAuthority() && PickupClass && NumPickups > 0)
	{
		SpawnPickUps();
		SetRotationActive(bRotate);
	}
	
}


void APCCarouseRing::SetRotationActive(bool bOn)
{
	bRotate = bOn;
	if (RotatingMovement)
		RotatingMovement->SetActive(bRotate);
}

void APCCarouseRing::SpawnPickUps()
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

void APCCarouseRing::ClearPickUps()
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


FTransform APCCarouseRing::GetSlotTransformWorld(int32 Index) const
{
	const int32 SlotNum = FMath::Max(1, NumSlots);
	const float Step = 360.f / SlotNum;
	const float Deg = StartAngleDeg + Step * (Index % SlotNum);
	const float Rad = FMath::DegreesToRadians(Deg);

	const FVector Local(FMath::Cos(Rad)*Radius, FMath::Sin(Rad)* Radius, Height);
	const FVector WorldPosition = GetActorTransform().TransformPosition(Local);

	FRotator Rotation = GetActorRotation();
	if (bFaceCenter)
	{
		Rotation = (GetActorLocation() - WorldPosition).Rotation();
	}
	return FTransform(Rotation, WorldPosition, FVector::OneVector);
}


#if WITH_EDITOR
void APCCarouseRing::OnConstruction(const FTransform& transform)
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
