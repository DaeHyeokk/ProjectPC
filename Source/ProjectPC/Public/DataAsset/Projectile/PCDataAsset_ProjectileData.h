// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PCDataAsset_ProjectileData.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCDataAsset_ProjectileData : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, Category = "Mesh")
	UStaticMesh* Mesh;

	UPROPERTY(EditDefaultsOnly, Category = "Effect")
	UParticleSystem* TrailEffect;
	
	UPROPERTY(EditDefaultsOnly, Category = "Effect")
	UParticleSystem* HitEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Speed")
	float Speed = 2000.f;

	UPROPERTY(EditDefaultsOnly, Category = "LifeTime")
	float LifeTime = 3.f;
};
