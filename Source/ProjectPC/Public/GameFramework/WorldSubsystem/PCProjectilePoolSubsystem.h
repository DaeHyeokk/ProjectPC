// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
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

	TQueue<APCBaseProjectile*> ProjectilePool;
	
public:
	UFUNCTION()
	void InitializeProjectilePoolData(const FPCProjectilePoolData& NewProjectilePoolData);

	UFUNCTION(BlueprintCallable)
	APCBaseProjectile* SpawnProjectile(const FTransform& SpawnTransform, FGameplayTag CharacterTag, FGameplayTag AttackTypeTag, const AActor* SpawnActor, const AActor* TargetActor, bool IsPlayerAttack = false);

	UFUNCTION(BlueprintCallable)
	void ReturnProjectile(APCBaseProjectile* ReturnedProjectile);
};
