// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Unit/EffectSpec/PCEffectSpec.h"
#include "DataAsset/Projectile/PCDataAsset_ProjectileData.h"
#include "GameFramework/Actor.h"
#include "PCBaseProjectile.generated.h"

class UArrowComponent;
class UProjectileMovementComponent;

UCLASS()
class PROJECTPC_API APCBaseProjectile : public AActor
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, Category = "ProjectileData")
	TMap<FGameplayTag, TObjectPtr<UPCDataAsset_ProjectileData>> ProjectileData;

	UPROPERTY()
	TArray<UPCEffectSpec*> EffectSpecs;
	
	UPROPERTY(ReplicatedUsing = OnRep_ProjectileDataTag)
	FGameplayTag ProjectileDataUnitTag;

	UPROPERTY(ReplicatedUsing = OnRep_ProjectileDataTag)
	FGameplayTag ProjectileDataTypeTag;
	
	UPROPERTY()
	UProjectileMovementComponent* ProjectileMovement;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	USceneComponent* RootComp;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UArrowComponent* ArrowComp;
	
	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* MeshComp;
	
	UPROPERTY(EditAnywhere)
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
	
private:
	UPROPERTY()
	const AActor* Target;

public:
	UFUNCTION(BlueprintCallable)
	void ActiveProjectile(const FTransform& SpawnTransform, FGameplayTag UnitTag, FGameplayTag TypeTag, const AActor* SpawnActor, const AActor* TargetActor);
	
	UFUNCTION(BlueprintCallable)
	void SetProjectileProperty();
	
	UFUNCTION(BlueprintCallable)
	void SetTarget(const AActor* TargetActor);

	UFUNCTION()
	void SetEffectSpecs(const TArray<UPCEffectSpec*>& InEffectSpecs);
	
	UFUNCTION(BlueprintCallable)
	void ReturnToPool();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
protected:
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
	
	UFUNCTION()
	void OnRep_ProjectileDataTag();

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_Overlap(AActor* OtherActor);
};
