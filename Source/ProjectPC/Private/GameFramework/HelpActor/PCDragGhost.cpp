// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFramework/HelpActor/PCDragGhost.h"


// Sets default values
APCDragGhost::APCDragGhost()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = false;

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	SetRootComponent(StaticMesh);
	StaticMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	StaticMesh->SetCastShadow(false);
	StaticMesh->SetReceivesDecals(false);
	StaticMesh->SetVisibility(false);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> Disc(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	if (Disc.Succeeded())
	{
		StaticMesh->SetStaticMesh(Disc.Object);
		StaticMesh->SetWorldScale3D(FVector(0.25f, 0.25f, 0.05f));
	}

}

void APCDragGhost::BeginPlay()
{
	Super::BeginPlay();

	if (UMaterialInterface* BaseMat = StaticMesh->GetMaterial(0))
	{
		MID = UMaterialInstanceDynamic::Create(BaseMat, this);
		StaticMesh->SetMaterial(0, MID);
	}
	ArmAutoDestroy();
	
}

APCDragGhost* APCDragGhost::SpawnGhost(UWorld* World, const FVector& SpawnAt)
{
	if (!World)
		return nullptr;
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	auto* Ghost = World->SpawnActor<APCDragGhost>(APCDragGhost::StaticClass(), SpawnAt, FRotator::ZeroRotator, SpawnInfo);
	return Ghost;
}

void APCDragGhost::UpdateState(bool bVisble, bool bValid, const FVector& Position)
{
	SetActorLocation(Position);
	if (StaticMesh)
	{
		StaticMesh->SetVisibility(bVisble);
	}

	if (MID)
	{
		const FLinearColor Color = bValid ? FLinearColor(0.2f,1,0.2f,0.6f) : FLinearColor(1,0.2f,0.2f,0.6f);
		// 머티리얼에 파라미터 없으면 SetVectorParameterValue는 그냥 무시됨
		MID->SetVectorParameterValue(TEXT("Color"), Color);
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AutoDestoryTimer);
		ArmAutoDestroy();
	}
}


void APCDragGhost::ArmAutoDestroy()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(AutoDestoryTimer, this, &APCDragGhost::DestroySelf, 5.0f,false);
	}
}



