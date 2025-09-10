// Fill out your copyright notice in the Description page of Project Settings.


#include "DataAsset/Unit/PCDataAsset_UnitAbilityConfig.h"


bool UPCDataAsset_UnitAbilityConfig::TryFindAbilityConfigByTag(const FGameplayTag& AbilityTag, FAbilityConfig& OutConfig) const
{
	if (AbilityConfigMap.Contains(AbilityTag))
	{
		return true;
	}
	else
		return false;
}
