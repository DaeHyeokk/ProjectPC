// Fill out your copyright notice in the Description page of Project Settings.


#include "DataAsset/Unit/PCDataAsset_HeroUnitData.h"

#include "BaseGameplayTags.h"
#include "GameplayEffect.h"
#include "Abilities/GameplayAbility.h"


UPCDataAsset_HeroUnitData::UPCDataAsset_HeroUnitData()
{
	HeroScalableStatConfigs.Emplace(UnitGameplayTags::Unit_Stat_Shared_MaxHealth);
	HeroScalableStatConfigs.Emplace(UnitGameplayTags::Unit_Stat_Shared_BaseDamage);

	HeroStaticStatConfigs.Emplace(UnitGameplayTags::Unit_Stat_Hero_MaxMana);
}

void UPCDataAsset_HeroUnitData::FillInitStatMap(int32 Level, TMap<FGameplayTag, float>& Out) const
{
	Super::FillInitStatMap(Level, Out);
	
	for (const FHeroScalableStatConfig& StatConfig : HeroScalableStatConfigs)
	{
		Out.Add(StatConfig.StatTag, StatConfig.StatValue.GetValueAtLevel(Level));
	}

	for (const FUnitStaticStatConfig& StatConfig : HeroStaticStatConfigs)
	{
		Out.Add(StatConfig.StatTag, StatConfig.StatValue);
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
