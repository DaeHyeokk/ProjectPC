// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFramework/HelpActor/Component/PCGoldDisplayComponent.h"


UPCGoldDisplayComponent::UPCGoldDisplayComponent()
{
	
	PrimaryComponentTick.bCanEverTick = false;
	SetMobility(EComponentMobility::Movable);
}


void UPCGoldDisplayComponent::OnRegister()
{
	Super::OnRegister();
}

void UPCGoldDisplayComponent::ReSetMyDisplay()
{
	for (UStaticMeshComponent* C : MyMeshPool)
	{
		if (C)
			C->DestroyComponent();
	}

	MyMeshPool.Reset();
}

void UPCGoldDisplayComponent::ReSetEnemyDisplay()
{
	EnsureEnemyPool(0);
	EnemyLayOutVisible(0);
	EnemyMeshPool.Reset();
}

void UPCGoldDisplayComponent::EnsureMyPool(int32 NeededCount)
{
	NeededCount = FMath::Clamp(NeededCount, 0, MaxPoolSize);
	while (MyMeshPool.Num() < NeededCount)
	{
		FName Name = *FString::Printf(TEXT("Gold_%02d"), MyMeshPool.Num());
		UStaticMeshComponent* C = NewObject<UStaticMeshComponent>(this, Name);
		C->SetupAttachment(this);
		C->SetMobility(EComponentMobility::Movable);
		C->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		C->SetGenerateOverlapEvents(false);
		C->bCastDynamicShadow = false;
		C->bCastStaticShadow = false;

		if (MyGoldMesh) C->SetStaticMesh(MyGoldMesh);
		C->RegisterComponent();

		MyMeshPool.Add(C);
	}
}

void UPCGoldDisplayComponent::EnsureEnemyPool(int32 NeededCount)
{
	NeededCount = FMath::Clamp(NeededCount, 0, MaxPoolSize);
	while (EnemyMeshPool.Num() < NeededCount)
	{
		FName Name = *FString::Printf(TEXT("Gold_%02d"), EnemyMeshPool.Num());
		UStaticMeshComponent* C = NewObject<UStaticMeshComponent>(this, Name);
		C->SetupAttachment(this);
		C->SetMobility(EComponentMobility::Movable);
		C->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		C->SetGenerateOverlapEvents(false);
		C->bCastDynamicShadow = false;
		C->bCastStaticShadow = false;

		if (EnemyGoldMesh) C->SetStaticMesh(EnemyGoldMesh);
		C->RegisterComponent();

		EnemyMeshPool.Add(C);
	}
}

void UPCGoldDisplayComponent::MyLayOutVisible(int32 VisibleCount)
{
	for (int32 i = 0; i < MyMeshPool.Num(); ++i)
	{
		UStaticMeshComponent* C = MyMeshPool[i];
		if (!C) continue;

		const bool bInVisible = (i < VisibleCount);
		C->SetVisibility(bInVisible, true);
		C->SetHiddenInGame(!bInVisible, true);

		if (bInVisible)
		{
			const FVector RelLoc = StartOffset + (Spacing * i);
			C->SetRelativeLocation(RelLoc);
			C->SetRelativeRotation(FRotator::ZeroRotator);
			C->SetRelativeScale3D(MeshScale);
		}
	}
}

void UPCGoldDisplayComponent::EnemyLayOutVisible(int32 VisibleCount)
{
	for (int32 i = 0; i < EnemyMeshPool.Num(); ++i)
	{
		UStaticMeshComponent* C = EnemyMeshPool[i];
		if (!C) continue;

		const bool bInVisible = (i < VisibleCount);
		C->SetVisibility(bInVisible, true);
		C->SetHiddenInGame(!bInVisible, true);

		if (bInVisible)
		{
			const FVector RelLoc = EnemyStartOffset + (EnemySpacing * i);
			C->SetRelativeLocation(RelLoc);
			C->SetRelativeRotation(EnemyRotator);
			C->SetRelativeScale3D(MeshScale);
		}
	}
}


void UPCGoldDisplayComponent::UpdateFromMyGold(int32 PlayerGold)
{
	const int32 Needed = FMath::Clamp(PlayerGold / FMath::Max(1, GoldPerMesh), 0, MaxPoolSize);
	EnsureMyPool(Needed);
	MyLayOutVisible(Needed);
}

void UPCGoldDisplayComponent::UpdateFromEnemyGold(int32 PlayerGold)
{
	const int32 Needed = FMath::Clamp(PlayerGold / FMath::Max(1, GoldPerMesh), 0, MaxPoolSize);
	EnsureEnemyPool(Needed);
	EnemyLayOutVisible(Needed);
}












