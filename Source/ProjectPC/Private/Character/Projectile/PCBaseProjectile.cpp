// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Projectile/PCBaseProjectile.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemGlobals.h"
#include "BaseGameplayTags.h"
#include "AbilitySystem/Player/GA/PCGameplayAbility_DamageEvent.h"
#include "Character/Unit/PCBaseUnitCharacter.h"
#include "Components/ArrowComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

#include "DataAsset/Projectile/PCDataAsset_ProjectileData.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/WorldSubsystem/PCProjectilePoolSubsystem.h"
#include "Net/UnrealNetwork.h"


APCBaseProjectile::APCBaseProjectile()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	bAlwaysRelevant = true;

	RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = RootComp;

	ArrowComp = CreateDefaultSubobject<UArrowComponent>(TEXT("ArrowComp"));
	ArrowComp->SetupAttachment(RootComponent);
	
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	MeshComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	MeshComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	MeshComp->SetupAttachment(RootComponent);
	MeshComp->SetRelativeRotation(FRotator(90.f, 0.f, 0.f));
	//MeshComp->SetRelativeLocationAndRotation(FVector(100.f,0.f,0.f), FRotator(0.f,-90.f,0.f));
	
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovement->UpdatedComponent = RootComponent;
	ProjectileMovement->InitialSpeed = 1000.f;
	ProjectileMovement->MaxSpeed = 1000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->ProjectileGravityScale = 0.f;
	
	TrailEffectComp = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("TrailEffect"));
	TrailEffectComp->SetupAttachment(RootComponent);
}

void APCBaseProjectile::BeginPlay()
{
	Super::BeginPlay();

	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	SetReplicateMovement(true);
}


void APCBaseProjectile::ActiveProjectile(const FTransform& SpawnTransform, FGameplayTag UnitTag, FGameplayTag TypeTag, const AActor* SpawnActor, const AActor* TargetActor, bool IsPlayerAttack)
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

		bIsPlayerAttack = IsPlayerAttack;
	}
}

void APCBaseProjectile::SetProjectileProperty()
{
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

void APCBaseProjectile::SetEffectSpecs(const TArray<UPCEffectSpec*>& InEffectSpecs)
{
	EffectSpecs = InEffectSpecs;
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

	if (Target)
	{
		Target = nullptr;
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

	if (AActor* InstigatorActor = GetInstigator())
	{
		if (OtherActor != InstigatorActor)
		{
			if (bIsHomingProjectile && OtherActor != Target) return;
		
			if (HitEffect)
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitEffect, OtherActor->GetActorLocation(), OtherActor->GetActorRotation());
				Multicast_Overlap(OtherActor);
			}

			if (bIsPlayerAttack)
			{
				if (auto InstigatorPawn = Cast<APawn>(InstigatorActor))
				{
					if (auto InstigatorPS = InstigatorPawn->GetPlayerState())
					{
						UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(InstigatorPS, PlayerGameplayTags::Player_Event_ProjectileHit, FGameplayEventData());
					}
				}
			}
			else
			{
				for (auto EffectSpec : EffectSpecs)
				{
					if (EffectSpec)
					{
						if (auto Unit = Cast<APCBaseUnitCharacter>(GetInstigator()))
						{
							if (auto ASC = Unit->GetAbilitySystemComponent())
							{
								EffectSpec->ApplyEffect(ASC, Target);
							}
						}
					}
				}
			}
			
			if (!bIsPenetrating)
			{
				ReturnToPool();
			}
		}
	}
}

void APCBaseProjectile::OnRep_ProjectileDataTag()
{
	SetProjectileProperty();
}

void APCBaseProjectile::Multicast_Overlap_Implementation(AActor* OtherActor)
{
	if (HitEffect && OtherActor)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitEffect, OtherActor->GetActorLocation(), OtherActor->GetActorRotation());
	}
}
