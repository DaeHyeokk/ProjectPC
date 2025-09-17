// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Projectile/PCBaseProjectile.h"

#include "DataAsset/Projectile/PCDataAsset_ProjectileData.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"


APCBaseProjectile::APCBaseProjectile()
{
	PrimaryActorTick.bCanEverTick = false;
	
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

void APCBaseProjectile::NotifyHit(class UPrimitiveComponent* MyComp, AActor* Other,
                                  class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal,
                                  FVector NormalImpulse, const FHitResult& Hit)
{
	if (HitEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitEffect, HitLocation, HitNormal.Rotation());
	}

	Destroy();
}

void APCBaseProjectile::SetProjectileProperty(const FPCProjectileData& ProjectileData)
{
	if (ProjectileData.Mesh)
	{
		Mesh->SetStaticMesh(ProjectileData.Mesh);
	}
	if (ProjectileData.TrailEffect)
	{
		TrailEffect->SetTemplate(ProjectileData.TrailEffect);
	}
	if (ProjectileData.HitEffect)
	{
		HitEffect = ProjectileData.HitEffect;
	}

	ProjectileMovement->InitialSpeed = ProjectileData.Speed;
	ProjectileMovement->MaxSpeed = ProjectileData.Speed;
	ProjectileMovement->Velocity = GetActorForwardVector();
	InitialLifeSpan = ProjectileData.LifeTime;
	bIsHomingProjectile = ProjectileData.bIsHomingProjectile;
	bIsPenetrating = ProjectileData.bIsPenetrating;
	
	if (bIsPenetrating)
	{
		Mesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	}
}

void APCBaseProjectile::SetTarget(AActor* TargetActor)
{
	if (ProjectileMovement && TargetActor)
	{
		if (bIsHomingProjectile)
		{
			UE_LOG(LogTemp, Error, TEXT("Homing Target : %s"), *TargetActor->GetName());
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
