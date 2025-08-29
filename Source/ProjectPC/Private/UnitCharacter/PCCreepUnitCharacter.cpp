// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/UnitCharacter/PCCreepUnitCharacter.h"

#include "BaseGameplayTags.h"
#include "AbilitySystem/Unit/PCUnitAbilitySystemComponent.h"
#include "AbilitySystem/Unit/AttributeSet/PCUnitAttributeSet.h"
#include "DataAsset/Unit/PCDataAsset_CreepUnitData.h"


UPCUnitAbilitySystemComponent* APCCreepUnitCharacter::GetUnitAbilitySystemComponent() const
{
	return UnitAbilitySystemComponent;
}

const UPCDataAsset_BaseUnitData* APCCreepUnitCharacter::GetUnitDataAsset() const
{
	return CreepUnitDataAsset;
}

FGameplayTag APCCreepUnitCharacter::GetUnitTypeTag() const
{
	return UnitGameplayTags::Unit_Type_Creep;
}

void APCCreepUnitCharacter::InitStatusBarWidget(UUserWidget* StatusBarWidget)
{
	// 데디서버거나 StatusBar Class가 없으면 실행하지 않음, HasAuthority() 안쓰는 이유: Listen Server 환경 고려
	if (GetNetMode() == NM_DedicatedServer || !CreepStatusBarClass)
		return;

	if (UPCUnitStatusBarWidget* StatusBar = Cast<UPCUnitStatusBarWidget>(StatusBarWidget))
	{
		StatusBar->InitWithASC(GetUnitAbilitySystemComponent(),
			UPCUnitAttributeSet::GetCurrentHealthAttribute(),
			UPCUnitAttributeSet::GetMaxHealthAttribute()
			);
		
	}
}
