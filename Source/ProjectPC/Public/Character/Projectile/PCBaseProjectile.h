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
	UPROPERTY()
	UProjectileMovementComponent* ProjectileMovement;
	
	UPROPERTY()
	UStaticMeshComponent* Mesh;
	
	UPROPERTY()
	UParticleSystemComponent* TrailEffect;
	
	UPROPERTY()
	UParticleSystem* HitEffect;

	bool bIsHomingProjectile = true;
	bool bIsPenetrating = false;

	UPROPERTY(ReplicatedUsing = OnRep_bIsUsing)
	bool bIsUsing = false;

	FTimerHandle LifeTimer;

	// 클라이언트 복제를 위한 캐싱
	UPROPERTY(ReplicatedUsing = OnRep_Mesh)
	UStaticMesh* RepMesh;

	UPROPERTY(ReplicatedUsing = OnRep_TrailEffect)
	UParticleSystem* RepTrailEffect;

	UPROPERTY(ReplicatedUsing = OnRep_HitEffect)
	UParticleSystem* RepHitEffect;
	
public:	
	APCBaseProjectile();

protected:
	virtual void BeginPlay() override;

public:
	FORCEINLINE void SetIsUsing(bool bNewUsing) { bIsUsing = bNewUsing; }
	FORCEINLINE bool GetIsUsing() const { return bIsUsing; }

	UFUNCTION(BlueprintCallable)
	void ActiveProjectile(const FTransform& SpawnTransform, const FPCProjectileData& ProjectileData, const AActor* TargetActor);
	
	UFUNCTION(BlueprintCallable)
	void SetProjectileProperty(const FPCProjectileData& ProjectileData);
	
	UFUNCTION(BlueprintCallable)
	void SetTarget(const AActor* TargetActor);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
protected:
	virtual void NotifyHit(class UPrimitiveComponent* MyComp, AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;

	void OnLifeTimeEnd();
	
	UFUNCTION()
	void OnRep_bIsUsing();
	UFUNCTION()
	void OnRep_Mesh();
	UFUNCTION()
	void OnRep_TrailEffect();
	UFUNCTION()
	void OnRep_HitEffect();
};
