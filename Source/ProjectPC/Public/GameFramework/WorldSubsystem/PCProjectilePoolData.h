// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PCProjectilePoolData.generated.h"

class APCBaseProjectile;
/**
 * 
 */
USTRUCT(BlueprintType)
struct PROJECTPC_API FPCProjectilePoolData
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, Category = "ObjectPool")
	TSubclassOf<APCBaseProjectile> ProjectileBaseClass;

	UPROPERTY(EditAnywhere, Category = "ObjectPool")
	int32 InitialReserveSize = 500;
};
