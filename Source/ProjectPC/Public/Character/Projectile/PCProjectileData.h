// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PCProjectileData.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct PROJECTPC_API FPCProjectileData
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, Category = "Mesh")
	TObjectPtr<UStaticMesh> Mesh = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Effect")
	TObjectPtr<UParticleSystem> TrailEffect = nullptr;
	
	UPROPERTY(EditDefaultsOnly, Category = "Effect")
	TObjectPtr<UParticleSystem> HitEffect = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Speed")
	float Speed = 2000.f;

	UPROPERTY(EditDefaultsOnly, Category = "LifeTime")
	float LifeTime = 3.f;

	UPROPERTY(EditDefaultsOnly, Category = "Property")
	bool bIsHomingProjectile = true;

	UPROPERTY(EditDefaultsOnly, Category = "Property")
	bool bIsPenetrating = false;
};
