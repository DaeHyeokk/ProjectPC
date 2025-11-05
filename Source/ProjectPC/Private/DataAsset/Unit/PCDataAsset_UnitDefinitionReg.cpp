// Fill out your copyright notice in the Description page of Project Settings.


#include "DataAsset/Unit/PCDataAsset_UnitDefinitionReg.h"

#include "BaseGameplayTags.h"


void UPCDataAsset_UnitDefinitionReg::GatherAllRegisteredHeroTags(FGameplayTagContainer& OutTags) const
{
	OutTags.Reset();
	for (const auto& Pair : UnitDefinitionByTagMap)
	{
		const FGameplayTag& UnitTag = Pair.Key;
		if (UnitTag.MatchesTag(UnitGameplayTags::Unit_Type_Hero))
		{
			OutTags.AddTag(UnitTag);
		}
	}
}
