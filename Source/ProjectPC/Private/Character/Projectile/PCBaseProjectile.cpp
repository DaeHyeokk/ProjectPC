// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Projectile/PCBaseProjectile.h"

#include "DataAsset/Projectile/PCDataAsset_ProjectileData.h"
#include "GameFramework/ProjectileMovementComponent.h"
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
	ProjectileMovement->InitialSpeed = 2000.f;
	ProjectileMovement->MaxSpeed = 2000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
	
	TrailEffect = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("TrailEffect"));
	TrailEffect->SetupAttachment(Mesh);
}

void APCBaseProjectile::BeginPlay()
{
	Super::BeginPlay();

	if (ProjectileData)
	{
		if (ProjectileData->Mesh)
		{
			Mesh->SetStaticMesh(ProjectileData->Mesh);
		}
		if (ProjectileData->TrailEffect)
		{
			TrailEffect->SetTemplate(ProjectileData->TrailEffect);
		}
		if (ProjectileData->HitEffect)
		{
			HitEffect = ProjectileData->HitEffect;
		}

		ProjectileMovement->InitialSpeed = ProjectileData->Speed;
		InitialLifeSpan = ProjectileData->LifeTime;
	}
}
