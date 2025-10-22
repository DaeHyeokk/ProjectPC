// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "PCItemSpawnSubsystem.generated.h"

class APCItemCapsule;
/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCItemSpawnSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	void InitializeItemCapsuleClass(TSubclassOf<APCItemCapsule> NewItemCapsuleClass);

private:
	TSubclassOf<APCItemCapsule> ItemCapsuleClass;
	
public:
	APCItemCapsule* SpawnItemCapsule(const FTransform& SpawnTransform, int32 TeamIndex);
};
