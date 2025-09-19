// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Projectile/PCBaseProjectile.h"

#include "DataAsset/Projectile/PCDataAsset_ProjectileData.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Particles/ParticleSystemComponent.h"


APCBaseProjectile::APCBaseProjectile()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	bAlwaysRelevant = true;
	
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	MeshComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	MeshComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	RootComponent = MeshComp;
	
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovement->UpdatedComponent = RootComponent;
	ProjectileMovement->InitialSpeed = 1000.f;
	ProjectileMovement->MaxSpeed = 1000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
	
	TrailEffectComp = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("TrailEffect"));
	TrailEffectComp->SetupAttachment(MeshComp);
}

void APCBaseProjectile::BeginPlay()
{
	Super::BeginPlay();

	SetReplicateMovement(true);
}

void APCBaseProjectile::ActiveProjectile(const FTransform& SpawnTransform, const FPCProjectileData& NewProjectileData, const AActor* TargetActor)
{
	bIsUsing = true;
	OnRep_bIsUsing();

	SetActorTransform(SpawnTransform);
	SetProjectileProperty(NewProjectileData);
	SetTarget(TargetActor);
}

void APCBaseProjectile::SetProjectileProperty(const FPCProjectileData& NewProjectileData)
{
	if (NewProjectileData.Mesh)
	{
		ProjectileData.Mesh = NewProjectileData.Mesh;
		MeshComp->SetStaticMesh(NewProjectileData.Mesh);
	}
	if (NewProjectileData.TrailEffect)
	{
		ProjectileData.TrailEffect = NewProjectileData.TrailEffect;
		TrailEffectComp->SetTemplate(NewProjectileData.TrailEffect);
	}
	if (NewProjectileData.HitEffect)
	{
		ProjectileData.HitEffect = NewProjectileData.HitEffect;
		HitEffect = NewProjectileData.HitEffect;
	}
	
	ProjectileMovement->InitialSpeed = NewProjectileData.Speed;
	ProjectileMovement->MaxSpeed = NewProjectileData.Speed;
	ProjectileMovement->Velocity = GetActorForwardVector();
	bIsHomingProjectile = NewProjectileData.bIsHomingProjectile;
	bIsPenetrating = NewProjectileData.bIsPenetrating;
	
	if (bIsPenetrating)
	{
		MeshComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	}
	
	if (NewProjectileData.LifeTime > 0.f)
	{
		GetWorldTimerManager().SetTimer(LifeTimer, this, &APCBaseProjectile::OnLifeTimeEnd, ProjectileData.LifeTime, false);
	}
}

void APCBaseProjectile::SetTarget(const AActor* TargetActor)
{
	if (ProjectileMovement && TargetActor)
	{
		if (bIsHomingProjectile)
		{
			ProjectileMovement->bIsHomingProjectile = true;
			ProjectileMovement->HomingAccelerationMagnitude = 15000.f;
			ProjectileMovement->HomingTargetComponent = TargetActor->GetRootComponent();
		}
		else
		{
			FVector TargetLocation = TargetActor->GetActorLocation();
			FVector Direction = (TargetLocation - GetActorLocation()).GetSafeNormal();
	
			ProjectileMovement->Velocity = Direction * ProjectileMovement->InitialSpeed;
		}
	}
}

void APCBaseProjectile::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(APCBaseProjectile, bIsUsing);
	DOREPLIFETIME(APCBaseProjectile, ProjectileData);
}

void APCBaseProjectile::OnRep_bIsUsing()
{
	SetActorHiddenInGame(!bIsUsing);
	SetActorEnableCollision(bIsUsing);
}

void APCBaseProjectile::NotifyHit(class UPrimitiveComponent* MyComp, AActor* Other,
                                  class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal,
                                  FVector NormalImpulse, const FHitResult& Hit)
{
	if (HitEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitEffect, HitLocation, HitNormal.Rotation());
	}

	bIsUsing = false;
	OnRep_bIsUsing();

	if (ProjectileMovement)
	{
		ProjectileMovement->StopMovementImmediately();
	}
}

void APCBaseProjectile::OnLifeTimeEnd()
{
	bIsUsing = false;
	OnRep_bIsUsing();

	if (ProjectileMovement)
	{
		ProjectileMovement->StopMovementImmediately();
	}

	GetWorldTimerManager().ClearTimer(LifeTimer);
}

void APCBaseProjectile::OnRep_ProjectileData()
{
	if (ProjectileData.Mesh)
	{
		MeshComp->SetStaticMesh(ProjectileData.Mesh);
	}
	if (ProjectileData.TrailEffect)
	{
		TrailEffectComp->SetTemplate(ProjectileData.TrailEffect);
	}
	if (ProjectileData.HitEffect)
	{
		HitEffect = ProjectileData.HitEffect;
	}
	
	ProjectileMovement->InitialSpeed = ProjectileData.Speed;
	ProjectileMovement->MaxSpeed = ProjectileData.Speed;
	ProjectileMovement->Velocity = GetActorForwardVector();
	bIsHomingProjectile = ProjectileData.bIsHomingProjectile;
	bIsPenetrating = ProjectileData.bIsPenetrating;
	
	if (bIsPenetrating)
	{
		MeshComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	}
	
	if (ProjectileData.LifeTime > 0.f)
	{
		GetWorldTimerManager().SetTimer(LifeTimer, this, &APCBaseProjectile::OnLifeTimeEnd, ProjectileData.LifeTime, false);
	}
}
