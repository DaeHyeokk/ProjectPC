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

	UPROPERTY(ReplicatedUsing = OnRep_bIsUsing)
	bool bIsUsing = false;

	FTimerHandle LifeTimer;
	
public:	
	APCBaseProjectile();

protected:
	virtual void BeginPlay() override;

public:
	FORCEINLINE void SetIsUsing(bool bNewUsing) { bIsUsing = bNewUsing; }
	FORCEINLINE bool GetIsUsing() const { return bIsUsing; }

	UFUNCTION(BlueprintCallable)
	void ActiveProjectile(const FTransform& SpawnTransform, const FPCProjectileData& NewProjectileData, const AActor* TargetActor);
	
	UFUNCTION(BlueprintCallable)
	void SetProjectileProperty(const FPCProjectileData& NewProjectileData);
	
	UFUNCTION(BlueprintCallable)
	void SetTarget(const AActor* TargetActor);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
protected:
	virtual void NotifyHit(class UPrimitiveComponent* MyComp, AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;

	void OnLifeTimeEnd();
	
	UFUNCTION()
	void OnRep_bIsUsing();
	UFUNCTION()
	void OnRep_ProjectileData();
};
