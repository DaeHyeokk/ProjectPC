// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DataAsset/Unit/PCDataAsset_UnitAbilityConfig.h"
#include "Engine/DataAsset.h"
#include "PCDataAsset_ItemEffect.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCDataAsset_ItemEffect : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FPCEffectSpecList EffectSpecList;
};