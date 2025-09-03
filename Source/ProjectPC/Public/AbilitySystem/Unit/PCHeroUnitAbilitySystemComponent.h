// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Unit/PCUnitAbilitySystemComponent.h"
#include "PCHeroUnitAbilitySystemComponent.generated.h"

class UPCDataAsset_BaseUnitData;
/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCHeroUnitAbilitySystemComponent : public UPCUnitAbilitySystemComponent
{
	GENERATED_BODY()

public:
	void UpdateGAS();
	
protected:
	virtual void GrantStartupAbilities() override;
	void UpdateBaseStatForLevel();
	void UpdateUltimateAbilityForLevel();
};
