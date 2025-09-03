// Fill out your copyright notice in the Description page of Project Settings.


#include "DataAsset/Unit/PCDataAsset_CreepUnitData.h"

#include "BaseGameplayTags.h"
#include "GameplayEffect.h"
#include "AbilitySystem/Unit/AttributeSet/PCUnitAttributeSet.h"


UPCDataAsset_CreepUnitData::UPCDataAsset_CreepUnitData()
{
	CreepStaticStatConfigs.Emplace(UPCUnitAttributeSet::GetMaxHealthAttribute());
	CreepStaticStatConfigs.Emplace(UPCUnitAttributeSet::GetBaseDamageAttribute());
}

void UPCDataAsset_CreepUnitData::FillInitStatMap(int32 Level, TMap<FGameplayAttribute, float>& Out) const
{
	Super::FillInitStatMap(Level, Out);
	
	for (const FUnitStaticStatConfig& StatConfig : CreepStaticStatConfigs)
	{
		Out.Add(StatConfig.StatAttribute, StatConfig.StatValue);
	}
}