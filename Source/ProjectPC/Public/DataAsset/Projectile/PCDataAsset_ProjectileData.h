// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Character/Projectile/PCProjectileData.h"
#include "PCDataAsset_ProjectileData.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCDataAsset_ProjectileData : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, Category = "ProjectileData")
	TMap<FGameplayTag, FPCProjectileData> ProjectileData;
};
