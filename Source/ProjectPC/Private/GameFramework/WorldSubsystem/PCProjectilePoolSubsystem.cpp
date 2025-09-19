// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFramework/WorldSubsystem/PCProjectilePoolSubsystem.h"

#include "Character/Projectile/PCBaseProjectile.h"
#include "Net/UnrealNetwork.h"


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
			Projectile->SetIsUsing(false);
			ProjectilePool.Add(Projectile);
		}
	}
}

void UPCProjectilePoolSubsystem::SpawnProjectile(const FTransform& SpawnTransform, const FPCProjectileData& ProjectileData, const AActor* TargetActor)
{
	if (!GetWorld() || GetWorld()->GetNetMode() == NM_Client) return;
	
	bool SpawnCheck = false;
	for (auto Projectile : ProjectilePool)
	{
		if (!Projectile->GetIsUsing())
		{
			Projectile->ActiveProjectile(SpawnTransform, ProjectileData, TargetActor);
			SpawnCheck = true;
			break;
		}
	}

	if (!SpawnCheck)
	{
		if (auto Projectile = GetWorld()->SpawnActor<APCBaseProjectile>(
			ProjectilePoolData.ProjectileBaseClass, FVector::ZeroVector, FRotator::ZeroRotator))
		{
			ProjectilePool.Add(Projectile);
			Projectile->ActiveProjectile(SpawnTransform, ProjectileData, TargetActor);
		}
	}
}
