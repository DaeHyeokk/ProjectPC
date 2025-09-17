// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PCBaseProjectile.generated.h"

class UProjectileMovementComponent;
struct FPCProjectileData;

UCLASS()
class PROJECTPC_API APCBaseProjectile : public AActor
{
	GENERATED_BODY()

protected:
	// UPROPERTY(EditDefaultsOnly, Category = "DataAsset")
	// class UPCDataAsset_ProjectileData* ProjectileData;
	
	UProjectileMovementComponent* ProjectileMovement;
	UStaticMeshComponent* Mesh;
	UParticleSystemComponent* TrailEffect;
	UParticleSystem* HitEffect;

	bool bIsHomingProjectile = true;
	bool bIsPenetrating = false;
	
public:	
	APCBaseProjectile();

protected:
	virtual void NotifyHit(class UPrimitiveComponent* MyComp, AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;

public:
	UFUNCTION(BlueprintCallable)
	void SetProjectileProperty(const FPCProjectileData& ProjectileData);
	
	UFUNCTION(BlueprintCallable)
	void SetTarget(AActor* TargetActor);
};
