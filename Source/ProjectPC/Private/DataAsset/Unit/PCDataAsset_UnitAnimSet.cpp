// Fill out your copyright notice in the Description page of Project Settings.


#include "DataAsset/Unit/PCDataAsset_UnitAnimSet.h"

#include "BaseGameplayTags.h"

FMontageConfig UPCDataAsset_UnitAnimSet::GetMontageConfigByTag(const FGameplayTag& MontageTag) const
{
	return MontageConfigByTagMap.FindRef(MontageTag);
}

UAnimMontage* UPCDataAsset_UnitAnimSet::GetMontageByTag(const FGameplayTag& MontageTag) const
{
	const FMontageConfig& MontageConfig = MontageConfigByTagMap.FindRef(MontageTag);
	return MontageConfig.Montage;
}
