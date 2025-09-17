// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PCBaseProjectile.generated.h"

class UProjectileMovementComponent;

UCLASS()
class PROJECTPC_API APCBaseProjectile : public AActor
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, Category = "DataAsset")
	class UPCDataAsset_ProjectileData* ProjectileData;
	
	UProjectileMovementComponent* ProjectileMovement;
	UStaticMeshComponent* Mesh;
	UParticleSystemComponent* TrailEffect;
	UParticleSystem* HitEffect;
	
public:	
	APCBaseProjectile();

protected:
	virtual void BeginPlay() override;
	
};
