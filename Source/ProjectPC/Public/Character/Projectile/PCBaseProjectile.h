// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PCProjectileData.h"
#include "GameFramework/Actor.h"
#include "PCBaseProjectile.generated.h"

class UProjectileMovementComponent;

UCLASS()
class PROJECTPC_API APCBaseProjectile : public AActor
{
	GENERATED_BODY()

protected:
	UPROPERTY(ReplicatedUsing = OnRep_ProjectileData)
	FPCProjectileData ProjectileData;
	
	UPROPERTY()
	UProjectileMovementComponent* ProjectileMovement;
	
	UPROPERTY()
	UStaticMeshComponent* MeshComp;
	
	UPROPERTY()
	UParticleSystemComponent* TrailEffectComp;
	
	UPROPERTY()
	UParticleSystem* HitEffect;
	
	bool bIsHomingProjectile = true;
	bool bIsPenetrating = false;

	FTimerHandle LifeTimer;
	
public:	
	APCBaseProjectile();

protected:
	virtual void BeginPlay() override;

public:
	UFUNCTION(BlueprintCallable)
	void ActiveProjectile(const FTransform& SpawnTransform, const FPCProjectileData& NewProjectileData, const AActor* TargetActor);
	
	UFUNCTION(BlueprintCallable)
	void SetProjectileProperty(const FPCProjectileData& NewProjectileData);
	
	UFUNCTION(BlueprintCallable)
	void SetTarget(const AActor* TargetActor);

	UFUNCTION(BlueprintCallable)
	void ReturnToPool();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
protected:
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

	void OnLifeTimeEnd();
	
	UFUNCTION()
	void OnRep_ProjectileData();
};
