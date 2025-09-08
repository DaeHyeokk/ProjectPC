// Fill out your copyright notice in the Description page of Project Settings.


#include "DataAsset/Unit/PCDataAsset_UnitAnimSet.h"


UAnimMontage* UPCDataAsset_UnitAnimSet::GetAnimMontageByTag(const FGameplayTag& MontageTag) const
{
	return MontageByTagMap.FindRef(MontageTag);
}

UAnimMontage* UPCDataAsset_UnitAnimSet::GetRandomBasicAttackMontage() const
{
	return nullptr;
}
