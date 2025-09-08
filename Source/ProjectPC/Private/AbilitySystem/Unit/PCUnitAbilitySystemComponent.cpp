// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Unit/PCUnitAbilitySystemComponent.h"

#include "BaseGameplayTags.h"
#include "Character/Unit/PCBaseUnitCharacter.h"
#include "DataAsset/Unit/PCDataAsset_BaseUnitData.h"
#include "GameFramework/WorldSubsystem/PCUnitGERegistrySubsystem.h"

void UPCUnitAbilitySystemComponent::InitGAS()
{
	// GE & GA 적용은 서버에서만 실행
	if (!GetOwner() || !GetOwner()->HasAuthority())
		return;
	
	ApplyInitBaseStat();
	GrantStartupAbilities();
}

void UPCUnitAbilitySystemComponent::ApplyInitBaseStat()
{
	// AttributeSet 스탯 변경은 서버에서만 실행
	if (!GetOwner() || !GetOwner()->HasAuthority() || bInitBaseStatsApplied)
		return;

	const APCBaseUnitCharacter* UnitCharacter = Cast<APCBaseUnitCharacter>(GetOwner());
	if (!UnitCharacter)
		return;

	const UPCDataAsset_BaseUnitData* UnitData = UnitCharacter->GetUnitDataAsset();
	const int32 UnitLevel = UnitCharacter->HasLevelSystem() ? UnitCharacter->GetUnitLevel() : 1;
	TMap<FGameplayAttribute, float> AttrMap;
	
	UnitData->FillInitStatMap(UnitLevel, AttrMap);

	for (const auto& KV : AttrMap)
	{
		SetNumericAttributeBase(KV.Key, KV.Value);
	}

	bInitBaseStatsApplied = true;
}

void UPCUnitAbilitySystemComponent::GrantStartupAbilities()
{
	// GA 능력 부여는 서버에서만 실행
	if (!GetOwner() || !GetOwner()->HasAuthority())
		return;
	
	const APCBaseUnitCharacter* UnitCharacter = Cast<APCBaseUnitCharacter>(GetOwner());
	if (!UnitCharacter)
		return;
	
	const UPCDataAsset_BaseUnitData* UnitData = UnitCharacter->GetUnitDataAsset();
	if (!UnitData)
		return;
	
	TArray<TSubclassOf<UGameplayAbility>> GrantAbilities;
	UnitData->FillStartupAbilities(GrantAbilities);

	for (const auto& GAClass : GrantAbilities)
	{
		if (!*GAClass || FindAbilitySpecFromClass(GAClass))
			continue;

		GiveAbility(FGameplayAbilitySpec(GAClass, 1, INDEX_NONE, GetOwner()));
	}
}