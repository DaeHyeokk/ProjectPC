// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/UnitCharacter/PCCreepUnitCharacter.h"

#include "BaseGameplayTags.h"
#include "DataAsset/Unit/PCDataAsset_CreepUnitData.h"


UPCUnitAbilitySystemComponent* APCCreepUnitCharacter::GetUnitAbilitySystemComponent() const
{
	return Super::GetUnitAbilitySystemComponent();
}

const UPCDataAsset_BaseUnitData* APCCreepUnitCharacter::GetUnitDataAsset() const
{
	return CreepUnitDataAsset;
}

FGameplayTag APCCreepUnitCharacter::GetUnitTypeTag() const
{
	return UnitGameplayTags::Unit_Type_Creep;
}
