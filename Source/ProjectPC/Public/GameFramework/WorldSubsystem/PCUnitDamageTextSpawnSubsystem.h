// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DynamicUIActor/PCUnitTakenDamageTextActor.h"
#include "Subsystems/WorldSubsystem.h"
#include "PCUnitDamageTextSpawnSubsystem.generated.h"

class APCUnitTakenDamageTextActor;
/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCUnitDamageTextSpawnSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

private:
	UPROPERTY()
	TSubclassOf<APCUnitTakenDamageTextActor> DamageTextActorClass = nullptr;

	int32 PrewarmCount = 20;

public:
	void InitDamageTextSpawnSubsystem(const TSoftClassPtr<APCUnitTakenDamageTextActor>& InDamageTextActorClass);
	
	UFUNCTION()
	void SpawnDamageText(
		const USceneComponent* AttachToComp,
		const FDamageTextInitParams& InitParams,
		AActor* Owner = nullptr,
		AActor* Instigator = nullptr);

	void ReturnToPool(APCUnitTakenDamageTextActor* DamageTextActor);
	
private:
	TQueue<APCUnitTakenDamageTextActor*> DamageTextPool;
	
	APCUnitTakenDamageTextActor* GetDamageTextActor();
	APCUnitTakenDamageTextActor* SpawnNewDamageTextActor() const;
};
