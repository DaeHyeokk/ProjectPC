// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Projectile/PCBaseProjectile.h"

#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

#include "DataAsset/Projectile/PCDataAsset_ProjectileData.h"
#include "GameFramework/WorldSubsystem/PCProjectilePoolSubsystem.h"
#include "Net/UnrealNetwork.h"


APCBaseProjectile::APCBaseProjectile()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	bAlwaysRelevant = true;
	
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	MeshComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	MeshComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	RootComponent = MeshComp;
	
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovement->UpdatedComponent = RootComponent;
	ProjectileMovement->InitialSpeed = 1000.f;
	ProjectileMovement->MaxSpeed = 1000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->ProjectileGravityScale = 0.f;
	
	TrailEffectComp = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("TrailEffect"));
	TrailEffectComp->SetupAttachment(MeshComp);
}

void APCBaseProjectile::BeginPlay()
{
	Super::BeginPlay();

	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	SetReplicateMovement(true);
}

void APCBaseProjectile::ActiveProjectile(const FTransform& SpawnTransform, FGameplayTag UnitTag, FGameplayTag TypeTag, const AActor* SpawnActor, const AActor* TargetActor)
{
	if (SpawnActor && TargetActor)
	{
		ProjectileDataUnitTag = UnitTag;
		ProjectileDataTypeTag = TypeTag;
		
		SetInstigator(SpawnActor->GetInstigator());
		
		SetActorHiddenInGame(false);
		SetActorEnableCollision(true);
		SetActorTransform(SpawnTransform);
	
		SetProjectileProperty();
		SetTarget(TargetActor);
		
		Target = TargetActor;
	}
}

void APCBaseProjectile::SetProjectileProperty()
{
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Error, TEXT("Client Set Property"));
	}
	if (auto Unit = ProjectileData.Find(ProjectileDataUnitTag))
	{
		auto Projectile = Unit->Get()->GetProjectileData(ProjectileDataTypeTag);
		if (Projectile.Mesh && MeshComp)
		{
			MeshComp->SetStaticMesh(Projectile.Mesh);
		}
		if (Projectile.TrailEffect && TrailEffectComp)
		{
			TrailEffectComp->SetTemplate(Projectile.TrailEffect);
			TrailEffectComp->SetActive(true);
		}
		if (Projectile.HitEffect)
		{
			HitEffect = Projectile.HitEffect;
		}

		ProjectileMovement->InitialSpeed = Projectile.Speed;
		ProjectileMovement->MaxSpeed = Projectile.Speed;
		ProjectileMovement->Velocity = GetActorForwardVector();
		bIsHomingProjectile = Projectile.bIsHomingProjectile;
		bIsPenetrating = Projectile.bIsPenetrating;
	
		if (Projectile.LifeTime > 0.f)
		{
			GetWorldTimerManager().SetTimer(LifeTimer, this, &APCBaseProjectile::ReturnToPool, Projectile.LifeTime, false);
		}
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

void APCBaseProjectile::ReturnToPool()
{
	GetWorldTimerManager().ClearTimer(LifeTimer);
	
	if (ProjectileMovement)
	{
		ProjectileMovement->StopMovementImmediately();
		ProjectileMovement->bIsHomingProjectile = false;
		ProjectileMovement->HomingTargetComponent = nullptr;
	}

	if (TrailEffectComp)
	{
		TrailEffectComp->SetActive(false);
	}
	
	SetInstigator(nullptr);
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);

	if (HasAuthority())
	{
		if (auto* ProjectilePoolSubsystem = GetWorld()->GetSubsystem<UPCProjectilePoolSubsystem>())
		{
			ProjectilePoolSubsystem->ReturnProjectile(this);
		}
	}
}

void APCBaseProjectile::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(APCBaseProjectile, ProjectileDataUnitTag);
	DOREPLIFETIME(APCBaseProjectile, ProjectileDataTypeTag);
}

void APCBaseProjectile::NotifyActorBeginOverlap(AActor* OtherActor)
{
	if (!HasAuthority()) return;
	Super::NotifyActorBeginOverlap(OtherActor);
	
	if (OtherActor != GetInstigator())
	{
		if (bIsHomingProjectile && OtherActor != Target)
		{
			return;
		}
		
		if (HitEffect)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitEffect, OtherActor->GetActorLocation(), OtherActor->GetActorRotation());
			Multicast_Overlap(OtherActor);
		}

		if (!bIsPenetrating)
		{
			ReturnToPool();
		}
	}
}

void APCBaseProjectile::OnRep_ProjectileDataTag()
{
	// if (auto Unit = ProjectileData.Find(ProjectileDataUnitTag))
	// {
	// 	auto Projectile = Unit->Get()->GetProjectileData(ProjectileDataTypeTag);
	// 	if (Projectile.Mesh && MeshComp)
	// 	{
	// 		MeshComp->SetStaticMesh(Projectile.Mesh);
	// 	}
	// 	if (Projectile.TrailEffect && TrailEffectComp)
	// 	{
	// 		TrailEffectComp->SetTemplate(Projectile.TrailEffect);
	// 		TrailEffectComp->SetActive(true);
	// 	}
	// 	if (Projectile.HitEffect)
	// 	{
	// 		HitEffect = Projectile.HitEffect;
	// 	}
	//
	// 	ProjectileMovement->InitialSpeed = Projectile.Speed;
	// 	ProjectileMovement->MaxSpeed = Projectile.Speed;
	// 	ProjectileMovement->Velocity = GetActorForwardVector();
	// 	bIsHomingProjectile = Projectile.bIsHomingProjectile;
	// 	bIsPenetrating = Projectile.bIsPenetrating;
	//
	// 	if (Projectile.LifeTime > 0.f)
	// 	{
	// 		GetWorldTimerManager().SetTimer(LifeTimer, this, &APCBaseProjectile::ReturnToPool, Projectile.LifeTime, false);
	// 	}
	// }

	SetProjectileProperty();
}

void APCBaseProjectile::Multicast_Overlap_Implementation(AActor* OtherActor)
{
	if (HitEffect && OtherActor)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitEffect, OtherActor->GetActorLocation(), OtherActor->GetActorRotation());
	}
}
