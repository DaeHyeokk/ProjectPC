// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameFramework/WorldSubsystem/PCProjectilePoolData.h"
#include "PCDataAsset_ProjectilePoolData.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCDataAsset_ProjectilePoolData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "ObjectPool")
	FPCProjectilePoolData ProjectilePoolData;
};
