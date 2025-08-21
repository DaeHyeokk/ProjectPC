// Fill out your copyright notice in the Description page of Project Settings.


#include "DataAsset/Unit/PCDataAsset_BaseUnitData.h"

#include "BaseGameplayTags.h"
#include "GameplayEffect.h"
#include "AbilitySystem/Unit/PCUnitAbilitySystemComponent.h"

UPCDataAsset_BaseUnitData::UPCDataAsset_BaseUnitData()
{
	UnitStaticStatConfigs.Emplace(UnitGameplayTags::Unit_Stat_Shared_AttackRange);
	UnitStaticStatConfigs.Emplace(UnitGameplayTags::Unit_Stat_Shared_AttackSpeed);
	UnitStaticStatConfigs.Emplace(UnitGameplayTags::Unit_Stat_Shared_PhysicalDefense);
	UnitStaticStatConfigs.Emplace(UnitGameplayTags::Unit_Stat_Shared_MagicDefense);
}

void UPCDataAsset_BaseUnitData::FillInitStatMap(int32 Level,
	TMap<FGameplayTag, float>& Out) const
{
	Out.Reset();
	
	for (const FUnitStaticStatConfig& StatConfig : UnitStaticStatConfigs)
	{
		Out.Add(StatConfig.StatTag, StatConfig.StatValue);
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

TSubclassOf<UGameplayEffect> UPCDataAsset_BaseUnitData::GetInitGEClass() const
{
	return InitGEClass;
}

TSubclassOf<UGameplayEffect> UPCDataAsset_BaseUnitData::GetGrantUnitTypeGEClass(FGameplayTag UnitTypeTag) const
{
	const TSubclassOf<UGameplayEffect>* UnitTypeGE = GrantUnitTypeGEClassMap.Find(UnitTypeTag);
	return *UnitTypeGE;
}
