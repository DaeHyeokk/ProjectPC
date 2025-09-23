// Fill out your copyright notice in the Description page of Project Settings.


#include "DataAsset/Unit/PCDataAsset_UnitAnimSet.h"

#include "BaseGameplayTags.h"

FMontageConfig UPCDataAsset_UnitAnimSet::GetMontageConfigByTag(const FGameplayTag& MontageTag) const
{
	return MontageConfigByTagMap.FindRef(MontageTag);
}

bool UPCDataAsset_UnitAnimSet::TryGetRandomBasicAttackMontageConfigByTag(FMontageConfig& OutConfig) const
{
	if (BasicAttackMontageConfigs.Num() > 0)
	{
		const int32 Idx = FMath::RandHelper(BasicAttackMontageConfigs.Num());
		OutConfig = BasicAttackMontageConfigs[Idx];
		return true;
	}
	UE_LOG(LogTemp, Warning,
		TEXT("BasicAttackMontageConfigs is empty on %s"),
		*GetName());

	OutConfig = FMontageConfig();
	return false;
}

UAnimMontage* UPCDataAsset_UnitAnimSet::GetMontageByTag(const FGameplayTag& MontageTag) const
{
	const FMontageConfig& MontageConfig = MontageConfigByTagMap.FindRef(MontageTag);
	return MontageConfig.Montage;
}
