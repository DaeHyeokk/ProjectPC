// Fill out your copyright notice in the Description page of Project Settings.


#include "DataAsset/Unit/PCDataAsset_BaseUnitData.h"

#include "BaseGameplayTags.h"
#include "GameplayEffect.h"
#include "AbilitySystem/Unit/PCUnitAbilitySystemComponent.h"
#include "AbilitySystem/Unit/AttributeSet/PCUnitAttributeSet.h"

UPCDataAsset_BaseUnitData::UPCDataAsset_BaseUnitData()
{
	UnitStaticStatConfigs.Emplace(UPCUnitAttributeSet::GetAttackRangeAttribute());
	UnitStaticStatConfigs.Emplace(UPCUnitAttributeSet::GetAttackSpeedAttribute());
	UnitStaticStatConfigs.Emplace(UPCUnitAttributeSet::GetPhysicalDefenseAttribute());
	UnitStaticStatConfigs.Emplace(UPCUnitAttributeSet::GetMagicDefenseAttribute());
}

void UPCDataAsset_BaseUnitData::FillInitStatMap(int32 Level,
	TMap<FGameplayAttribute, float>& Out) const
{
	Out.Reset();
	
	for (const FUnitStaticStatConfig& StatConfig : UnitStaticStatConfigs)
	{
		Out.Add(StatConfig.StatAttribute, StatConfig.StatValue);
	}
}

void UPCDataAsset_BaseUnitData::FillStartupAbilities(TArray<TSubclassOf<UGameplayAbility>>& OutAbilities) const
{
	OutAbilities.Reset(
		AttackAbilities.Num() + PassiveAbilities.Num() +
				MovementAbilities.Num() + MiscAbilities.Num() + EventAbilities.Num());

	auto AppendValidUnique = [&OutAbilities](const TArray<TSubclassOf<UGameplayAbility>>& Src)
	{
		for (const TSubclassOf<UGameplayAbility>& GAClass : Src)
		{
			if (*GAClass)
			{
				OutAbilities.AddUnique(GAClass);
			}
		}
	};

	AppendValidUnique(AttackAbilities);
	AppendValidUnique(PassiveAbilities);
	AppendValidUnique(MovementAbilities);
	AppendValidUnique(MiscAbilities);
	AppendValidUnique(EventAbilities);
}

UPCDataAsset_UnitAnimSet* UPCDataAsset_BaseUnitData::GetAnimSetData() const
{
	return AnimSetData;
}

UPCDataAsset_UnitAbilityConfig* UPCDataAsset_BaseUnitData::GetAbilityConfigData() const
{
	return AbilityConfigData;
}
