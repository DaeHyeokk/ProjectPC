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

public:	
	APCBaseProjectile();

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

private:
	FTimerHandle LifeTimer;

public:
	// 유닛 to 유닛
	void ActiveProjectile(const FTransform& SpawnTransform, FGameplayTag CharacterTag, FGameplayTag AttackTypeTag, const AActor* SpawnActor, const AActor* TargetActor);
	// 유닛 to 플레이어
	void ActiveProjectile(const FTransform& SpawnTransform, const AActor* SpawnActor, const AActor* TargetActor);
	// 플레이어 to 플레이어
	void ActiveProjectile(const FTransform& SpawnTransform, FGameplayTag CharacterTag, const AActor* SpawnActor, const AActor* TargetActor);
	// 발사체 오브젝트 풀에 반환
	void ReturnToPool();
	
	void SetProjectileProperty();
	void SetTarget(const AActor* TargetActor);
	void SetEffectSpecs(const TArray<UPCEffectSpec*>& InEffectSpecs);
	void SetDamage(float InDamage);

private:
	UFUNCTION()
	void OnRep_ProjectileDataTag();

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_Overlap(AActor* OtherActor);

#pragma region ProjectileData
	
protected:
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
	
	UPROPERTY(EditDefaultsOnly, Category = "ProjectileData")
	TMap<FGameplayTag, TObjectPtr<UPCDataAsset_ProjectileData>> ProjectileData;
	
	UPROPERTY(ReplicatedUsing = OnRep_ProjectileDataTag)
	FGameplayTag ProjectileDataCharacterTag;

	UPROPERTY(ReplicatedUsing = OnRep_ProjectileDataTag)
	FGameplayTag ProjectileDataAttackTypeTag;

private:
	UPROPERTY()
	const AActor* Target;
	
	UPROPERTY()
	TArray<UPCEffectSpec*> EffectSpecs;

	float PlayerDamage = 0.f;
	
	bool bIsHomingProjectile = true;
	bool bIsPenetrating = false;
	bool bIsPlayerAttack = false;

#pragma endregion ProjectileData
};
