// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DataAsset/Unit/PCDataAsset_BaseUnitData.h"
#include "PCDataAsset_CreepUnitData.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCDataAsset_CreepUnitData : public UPCDataAsset_BaseUnitData
{
	GENERATED_BODY()

public:
	UPCDataAsset_CreepUnitData();
	virtual void FillInitStatMap(int32 Level, TMap<FGameplayAttribute, float>& Out) const override;
	
protected:
	UPROPERTY(EditDefaultsOnly, Category="Stats|Static")
	TArray<FUnitStaticStatConfig> CreepStaticStatConfigs;
};
