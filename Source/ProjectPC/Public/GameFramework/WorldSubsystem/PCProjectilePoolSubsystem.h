// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PCProjectilePoolData.h"
#include "Subsystems/WorldSubsystem.h"
#include "PCProjectilePoolSubsystem.generated.h"

struct FPCProjectileData;
struct FPCProjectilePoolData;

/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCProjectilePoolSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
protected:
	UPROPERTY()
	FPCProjectilePoolData ProjectilePoolData;

	UPROPERTY()
	TArray<APCBaseProjectile*> ProjectilePool;
	
public:
	UFUNCTION()
	void InitializeProjectilePoolData(const FPCProjectilePoolData& NewProjectilePoolData);

	UFUNCTION(BlueprintCallable)
	void SpawnProjectile(const FTransform& SpawnTransform, const FPCProjectileData& ProjectileData, const AActor* TargetActor);
};
