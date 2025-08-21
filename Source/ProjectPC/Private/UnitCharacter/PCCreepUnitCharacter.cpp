// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/UnitCharacter/PCCreepUnitCharacter.h"

#include "BaseGameplayTags.h"


UPCUnitAbilitySystemComponent* APCCreepUnitCharacter::GetUnitAbilitySystemComponent() const
{
	return Super::GetUnitAbilitySystemComponent();
}

FGameplayTag APCCreepUnitCharacter::GetUnitTypeTag() const
{
	return UnitGameplayTags::Unit_Type_Creep;
}
