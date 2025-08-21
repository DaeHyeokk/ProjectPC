// Fill out your copyright notice in the Description page of Project Settings.


#include "DataAsset/Unit/PCDataAsset_CreepUnitData.h"

#include "BaseGameplayTags.h"
#include "GameplayEffect.h"


UPCDataAsset_CreepUnitData::UPCDataAsset_CreepUnitData()
{
	CreepStaticStatConfigs.Emplace(UnitGameplayTags::Unit_Stat_Shared_MaxHealth);
	CreepStaticStatConfigs.Emplace(UnitGameplayTags::Unit_Stat_Shared_BaseDamage);
}

void UPCDataAsset_CreepUnitData::FillInitStatMap(int32 Level, TMap<FGameplayTag, float>& Out) const
{
	Super::FillInitStatMap(Level, Out);
	
	for (const FUnitStaticStatConfig& StatConfig : CreepStaticStatConfigs)
	{
		Out.Add(StatConfig.StatTag, StatConfig.StatValue);
	}
}