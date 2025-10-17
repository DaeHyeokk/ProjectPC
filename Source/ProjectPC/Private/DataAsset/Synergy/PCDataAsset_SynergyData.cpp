// Fill out your copyright notice in the Description page of Project Settings.


#include "DataAsset/Synergy/PCDataAsset_SynergyData.h"


int32 UPCDataAsset_SynergyData::ComputeActiveTierIndex(int32 CurrentCount) const
{
	if (CurrentCount <= 0 || Tiers.Num() == 0)
		return -1;

	int32 ActiveIdx = -1;
	for (int32 i=0; i<Tiers.Num(); ++i)
	{
		if (CurrentCount >= Tiers[i].Threshold)
			ActiveIdx = i;
		else
			break;
	}

	return ActiveIdx;
}

const FSynergyTier* UPCDataAsset_SynergyData::GetTier(int32 TierIndex) const
{
	return Tiers.IsValidIndex(TierIndex) ? &Tiers[TierIndex] : nullptr;
}
