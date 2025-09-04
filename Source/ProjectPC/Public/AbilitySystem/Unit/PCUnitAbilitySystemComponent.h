// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "DataAsset/Unit/PCDataAsset_BaseUnitData.h"
#include "PCUnitAbilitySystemComponent.generated.h"

class UPCDataAsset_BaseUnitData;
/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCUnitAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()
	
protected:
	bool bInitBaseStatsApplied = false;
	
public:
	virtual void InitGAS();
	
protected:
	virtual void ApplyInitBaseStat();
	virtual void GrantStartupAbilities();
};
