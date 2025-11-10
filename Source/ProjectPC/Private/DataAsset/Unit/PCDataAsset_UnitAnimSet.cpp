// Fill out your copyright notice in the Description page of Project Settings.


#include "DataAsset/Unit/PCDataAsset_UnitAnimSet.h"

#include "BaseGameplayTags.h"

UAnimMontage* UPCDataAsset_UnitAnimSet::GetMontageByTag(const FGameplayTag& MontageTag) const
{
	return MontageByTagMap.FindRef(MontageTag);
}

UAnimMontage* UPCDataAsset_UnitAnimSet::GetRandomBasicAttackMontage() const
{
	if (BasicAttackMontages.Num() > 0)
	{
		const int32 Idx = FMath::RandHelper(BasicAttackMontages.Num());
		return BasicAttackMontages[Idx];
	}
	UE_LOG(LogTemp, Warning,
		TEXT("BasicAttackMontageConfigs is empty on %s"),
		*GetName());

	return nullptr;
}