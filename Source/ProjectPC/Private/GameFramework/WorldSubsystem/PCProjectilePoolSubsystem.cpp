// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFramework/WorldSubsystem/PCProjectilePoolSubsystem.h"


#include "Character/Projectile/PCBaseProjectile.h"


void UPCProjectilePoolSubsystem::InitializeProjectilePoolData(const FPCProjectilePoolData& NewProjectilePoolData)
{
	if (!GetWorld() || GetWorld()->GetNetMode() == NM_Client) return;
	
	ProjectilePoolData = NewProjectilePoolData;

	if (!ProjectilePoolData.ProjectileBaseClass) return;

	for (int32 i=0; i<ProjectilePoolData.InitialReserveSize; ++i)
	{
		if (auto Projectile = GetWorld()->SpawnActor<APCBaseProjectile>(
			ProjectilePoolData.ProjectileBaseClass, FVector::ZeroVector, FRotator::ZeroRotator))
		{
			ProjectilePool.Enqueue(Projectile);
		}
	}
}

void UPCProjectilePoolSubsystem::SpawnProjectile(const FTransform& SpawnTransform, FGameplayTag UnitTag, FGameplayTag TypeTag, const AActor* SpawnActor, const AActor* TargetActor)
{
	if (!GetWorld() || GetWorld()->GetNetMode() == NM_Client) return;
	if (!SpawnActor || !TargetActor) return;

	APCBaseProjectile* SpawnedProjectile = nullptr;
	if (!ProjectilePool.IsEmpty() && ProjectilePool.Dequeue(SpawnedProjectile))
	{
		SpawnedProjectile->ActiveProjectile(SpawnTransform, UnitTag, TypeTag, SpawnActor, TargetActor);
	}
	else
	{
		if (SpawnedProjectile = GetWorld()->SpawnActor<APCBaseProjectile>(
			ProjectilePoolData.ProjectileBaseClass, FVector::ZeroVector, FRotator::ZeroRotator))
		{
			SpawnedProjectile->ActiveProjectile(SpawnTransform, UnitTag, TypeTag, SpawnActor, TargetActor);
		}
	}
}

void UPCProjectilePoolSubsystem::ReturnProjectile(APCBaseProjectile* ReturnedProjectile)
{
	if (!GetWorld() || GetWorld()->GetNetMode() == NM_Client) return;
	
	if (ReturnedProjectile)
	{
		ProjectilePool.Enqueue(ReturnedProjectile);
	}
}