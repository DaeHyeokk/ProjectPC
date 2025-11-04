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
		// InitialReserveSize만큼 발사체 미리 생성하고, Queue에 삽입
 		if (auto Projectile = GetWorld()->SpawnActor<APCBaseProjectile>(
			ProjectilePoolData.ProjectileBaseClass, FVector::ZeroVector, FRotator::ZeroRotator))
		{
			ProjectilePool.Enqueue(Projectile);
		}
	}
}

APCBaseProjectile* UPCProjectilePoolSubsystem::SpawnProjectile(const FTransform& SpawnTransform, FGameplayTag CharacterTag, FGameplayTag AttackTypeTag, const AActor* SpawnActor, const AActor* TargetActor)
{
	if (GetWorld() && GetWorld()->GetNetMode() != NM_Client)
	{
		if (SpawnActor && TargetActor)
		{
			APCBaseProjectile* SpawnedProjectile = nullptr;
			if (!ProjectilePool.IsEmpty() && ProjectilePool.Dequeue(SpawnedProjectile))
			{
				// 오브젝트 풀에서 꺼낼 수 있으면 즉시 활성화
				SpawnedProjectile->ActiveProjectile(SpawnTransform, CharacterTag, AttackTypeTag, SpawnActor, TargetActor);
				return SpawnedProjectile;
			}
			else
			{
				// 오브젝트 풀이 비었거나 꺼낸 발사체가 nullptr일 경우 새로 생성
				if (SpawnedProjectile = GetWorld()->SpawnActor<APCBaseProjectile>(
					ProjectilePoolData.ProjectileBaseClass, FVector::ZeroVector, FRotator::ZeroRotator))
				{
					SpawnedProjectile->ActiveProjectile(SpawnTransform, CharacterTag, AttackTypeTag, SpawnActor, TargetActor);
					return SpawnedProjectile;
				}
			}
		}
	}
	
	return nullptr;
}

APCBaseProjectile* UPCProjectilePoolSubsystem::SpawnProjectile(const FTransform& SpawnTransform,
	const AActor* SpawnActor, const AActor* TargetActor)
{
	if (GetWorld() && GetWorld()->GetNetMode() != NM_Client)
	{
		if (SpawnActor && TargetActor)
		{
			APCBaseProjectile* SpawnedProjectile = nullptr;
			if (!ProjectilePool.IsEmpty() && ProjectilePool.Dequeue(SpawnedProjectile))
			{
				SpawnedProjectile->ActiveProjectile(SpawnTransform, SpawnActor, TargetActor);
				return SpawnedProjectile;
			}
			else
			{
				if (SpawnedProjectile = GetWorld()->SpawnActor<APCBaseProjectile>(
					ProjectilePoolData.ProjectileBaseClass, FVector::ZeroVector, FRotator::ZeroRotator))
				{
					SpawnedProjectile->ActiveProjectile(SpawnTransform, SpawnActor, TargetActor);
					return SpawnedProjectile;
				}
			}
		}
	}
	
	return nullptr;
}

APCBaseProjectile* UPCProjectilePoolSubsystem::SpawnProjectile(const FTransform& SpawnTransform,
	FGameplayTag CharacterTag, const AActor* SpawnActor, const AActor* TargetActor)
{
	if (GetWorld() && GetWorld()->GetNetMode() != NM_Client)
	{
		if (SpawnActor && TargetActor)
		{
			APCBaseProjectile* SpawnedProjectile = nullptr;
			if (!ProjectilePool.IsEmpty() && ProjectilePool.Dequeue(SpawnedProjectile))
			{
				SpawnedProjectile->ActiveProjectile(SpawnTransform, CharacterTag, SpawnActor, TargetActor);
				return SpawnedProjectile;
			}
			else
			{
				if (SpawnedProjectile = GetWorld()->SpawnActor<APCBaseProjectile>(
					ProjectilePoolData.ProjectileBaseClass, FVector::ZeroVector, FRotator::ZeroRotator))
				{
					SpawnedProjectile->ActiveProjectile(SpawnTransform, CharacterTag, SpawnActor, TargetActor);
					return SpawnedProjectile;
				}
			}
		}
	}
	
	return nullptr;
}

void UPCProjectilePoolSubsystem::ReturnProjectile(APCBaseProjectile* ReturnedProjectile)
{
	if (!GetWorld() || GetWorld()->GetNetMode() == NM_Client) return;

	if (ReturnedProjectile)
	{
		// 반환된 발사체 오브젝트 풀 Queue에 삽입
		ProjectilePool.Enqueue(ReturnedProjectile);
	}
}