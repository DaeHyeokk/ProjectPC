// Fill out your copyright notice in the Description page of Project Settings.


#include "DataAsset/Unit/PCDataAsset_HeroUnitData.h"

#include "GameplayEffect.h"
#include "Abilities/GameplayAbility.h"
#include "AbilitySystem/Unit/AttributeSet/PCHeroUnitAttributeSet.h"
#include "AbilitySystem/Unit/AttributeSet/PCUnitAttributeSet.h"


UPCDataAsset_HeroUnitData::UPCDataAsset_HeroUnitData()
{
	HeroScalableStatConfigs.Emplace(UPCUnitAttributeSet::GetMaxHealthAttribute());
	HeroScalableStatConfigs.Emplace(UPCUnitAttributeSet::GetBaseDamageAttribute());
	HeroScalableStatConfigs.Emplace(UPCHeroUnitAttributeSet::GetUltimateDamageAttribute());
	
	HeroStaticStatConfigs.Emplace(UPCHeroUnitAttributeSet::GetUltimateCostAttribute());
	HeroStaticStatConfigs.Emplace(UPCHeroUnitAttributeSet::GetCurrentManaAttribute());
	HeroStaticStatConfigs.Emplace(UPCHeroUnitAttributeSet::GetMaxManaAttribute());
	HeroStaticStatConfigs.Emplace(UPCHeroUnitAttributeSet::GetCritChanceAttribute(), 25.f);
	HeroStaticStatConfigs.Emplace(UPCHeroUnitAttributeSet::GetCritMultiplierAttribute(), 0.3f);
}

void UPCDataAsset_HeroUnitData::FillInitStatMap(int32 Level, TMap<FGameplayAttribute, float>& Out) const
{
	Super::FillInitStatMap(Level, Out);
	
	for (const FHeroScalableStatConfig& StatConfig : HeroScalableStatConfigs)
	{
		Out.Add(StatConfig.StatAttribute, StatConfig.StatValue.GetValueAtLevel(Level));
	}

	for (const FUnitStaticStatConfig& StatConfig : HeroStaticStatConfigs)
	{
		Out.Add(StatConfig.StatAttribute, StatConfig.StatValue);
	}
}

void UPCDataAsset_HeroUnitData::FillStartupUltimateAbilities(TArray<TSubclassOf<UGameplayAbility>>& OutAbilities) const
{
	if (UltimateAttackAbility)
	{
		OutAbilities.AddUnique(UltimateAttackAbility);
	}
}

FGameplayTag UPCDataAsset_HeroUnitData::GetJobSynergyTag() const
{
	return SynergyTagConfig.JobSynergyTag;
}

FGameplayTag UPCDataAsset_HeroUnitData::GetSpeciesSynergyTag() const
{
	return SynergyTagConfig.SpeciesSynergyTag;
}

float UPCDataAsset_HeroUnitData::GetDefaultCurrentMana() const
{
	for (const FUnitStaticStatConfig& StatConfig : HeroStaticStatConfigs)
	{
		if (StatConfig.StatAttribute == UPCHeroUnitAttributeSet::GetCurrentManaAttribute())
			return StatConfig.StatValue;
	}

	return 0.f;
}
