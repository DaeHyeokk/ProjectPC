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
	
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Mesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	Mesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	RootComponent = Mesh;
	
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovement->UpdatedComponent = RootComponent;
	ProjectileMovement->InitialSpeed = 1000.f;
	ProjectileMovement->MaxSpeed = 1000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
	
	TrailEffect = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("TrailEffect"));
	TrailEffect->SetupAttachment(Mesh);
}

void APCBaseProjectile::BeginPlay()
{
	Super::BeginPlay();

	SetReplicateMovement(true);
}

void APCBaseProjectile::ActiveProjectile(const FTransform& SpawnTransform, const FPCProjectileData& ProjectileData, const AActor* TargetActor)
{
	bIsUsing = true;
	OnRep_bIsUsing();

	SetActorTransform(SpawnTransform);
	SetProjectileProperty(ProjectileData);
	SetTarget(TargetActor);
}

void APCBaseProjectile::SetProjectileProperty(const FPCProjectileData& ProjectileData)
{
	if (ProjectileData.Mesh)
	{
		RepMesh = ProjectileData.Mesh;
		Mesh->SetStaticMesh(ProjectileData.Mesh);
	}
	if (ProjectileData.TrailEffect)
	{
		RepTrailEffect = ProjectileData.TrailEffect;
		TrailEffect->SetTemplate(ProjectileData.TrailEffect);
	}
	if (ProjectileData.HitEffect)
	{
		RepHitEffect = ProjectileData.HitEffect;
		HitEffect = ProjectileData.HitEffect;
	}

	ProjectileMovement->InitialSpeed = ProjectileData.Speed;
	ProjectileMovement->MaxSpeed = ProjectileData.Speed;
	ProjectileMovement->Velocity = GetActorForwardVector();
	bIsHomingProjectile = ProjectileData.bIsHomingProjectile;
	bIsPenetrating = ProjectileData.bIsPenetrating;
	
	if (bIsPenetrating)
	{
		Mesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	}

	if (ProjectileData.LifeTime > 0.f)
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
	DOREPLIFETIME(APCBaseProjectile, RepMesh);
	DOREPLIFETIME(APCBaseProjectile, RepTrailEffect);
	DOREPLIFETIME(APCBaseProjectile, RepHitEffect);
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

void APCBaseProjectile::OnRep_Mesh()
{
	if (RepMesh)
	{
		Mesh->SetStaticMesh(RepMesh);
	}
}

void APCBaseProjectile::OnRep_TrailEffect()
{
	if (RepTrailEffect)
	{
		TrailEffect->SetTemplate(RepTrailEffect);
	}
}

void APCBaseProjectile::OnRep_HitEffect()
{
	if (RepHitEffect)
	{
		HitEffect = RepHitEffect;
	}
}
