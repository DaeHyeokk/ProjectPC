// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Unit/GA/PCUnitBasicAttackGameplayAbility.h"

#include "Character/Unit/PCBaseUnitCharacter.h"
#include "DataAsset/Unit/PCDataAsset_UnitAnimSet.h"

UAnimMontage* UPCUnitBasicAttackGameplayAbility::GetMontage(const FGameplayAbilityActorInfo* ActorInfo) const
{
	const APCBaseUnitCharacter* UnitCharacter = Cast<APCBaseUnitCharacter>(ActorInfo->OwnerActor.Get());
	const UPCDataAsset_UnitAnimSet* UnitAnimSet = UnitCharacter ? UnitCharacter->GetUnitAnimSetDataAsset() : nullptr;
	if (!UnitAnimSet)
		return nullptr;

	return UnitAnimSet->GetRandomBasicAttackMontage();
}
