// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "DataAsset/Unit/PCDataAsset_BaseUnitData.h"
#include "PCUnitAbilitySystemComponent.generated.h"

class APCBaseUnitCharacter;
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
	virtual void ApplyInitBaseStat(const APCBaseUnitCharacter* Unit, const UPCDataAsset_BaseUnitData* UnitData);
	virtual void GrantStartupAbilities(UPCDataAsset_BaseUnitData* UnitData);
	void GrantSynergyTags(const UPCDataAsset_BaseUnitData* UnitData);
};
