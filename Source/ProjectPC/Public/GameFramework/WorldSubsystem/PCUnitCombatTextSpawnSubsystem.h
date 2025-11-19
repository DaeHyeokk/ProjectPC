// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DynamicUIActor/PCUnitCombatTextActor.h"
#include "Subsystems/WorldSubsystem.h"
#include "PCUnitCombatTextSpawnSubsystem.generated.h"

class APCUnitCombatTextActor;
/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCUnitCombatTextSpawnSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

private:
	UPROPERTY()
	TSubclassOf<APCUnitCombatTextActor> CombatTextActorClass = nullptr;

	int32 PrewarmCount = 20;

public:
	void InitCombatTextSpawnSubsystem(const TSoftClassPtr<APCUnitCombatTextActor>& InDamageTextActorClass);
	
	UFUNCTION()
	void SpawnCombatText(
		const USceneComponent* AttachToComp,
		const FCombatTextInitParams& InitParams);

	void ReturnToPool(APCUnitCombatTextActor* CombatTextActor);
	
private:
	TQueue<APCUnitCombatTextActor*> CombatTextPool;
	
	APCUnitCombatTextActor* GetCombatTextActor();
	APCUnitCombatTextActor* SpawnNewCombatTextActor() const;
};
