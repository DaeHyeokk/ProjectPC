// Fill out your copyright notice in the Description page of Project Settings.


#include "DataAsset/Unit/PCDataAsset_UnitAnimSet.h"

#include "BaseGameplayTags.h"


UAnimMontage* UPCDataAsset_UnitAnimSet::GetAnimMontageByTag(const FGameplayTag& MontageTag) const
{
	// 기본 공격일 경우 여러가지 공격 모션 중 하나 재생
	if (MontageTag.MatchesTag(UnitGameplayTags::Unit_Action_Attack_Basic))
	{
		return GetRandomBasicAttackMontage();
	}
	
	return MontageByTagMap.FindRef(MontageTag);
}

UAnimMontage* UPCDataAsset_UnitAnimSet::GetRandomBasicAttackMontage() const
{
	if (!BasicAttackMontages.IsEmpty())
	{
		const int32 Index = FMath::RandHelper(BasicAttackMontages.Num());
		return BasicAttackMontages[Index];
	}

	return nullptr;
}
