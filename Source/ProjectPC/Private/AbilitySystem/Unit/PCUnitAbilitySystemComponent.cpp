// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Unit/PCUnitAbilitySystemComponent.h"

#include "AbilitySystem/Unit/AttributeSet/PCHeroUnitAttributeSet.h"
#include "Character/Unit/PCBaseUnitCharacter.h"
#include "DataAsset/Unit/PCDataAsset_UnitAbilityConfig.h"
#include "DataAsset/Unit/PCDataAsset_BaseUnitData.h"
#include "GameFramework/WorldSubsystem/PCUnitGERegistrySubsystem.h"

void UPCUnitAbilitySystemComponent::InitGAS()
{
	// GE & GA 적용은 서버에서만 실행
	if (!GetOwner() || !GetOwner()->HasAuthority())
		return;

	const APCBaseUnitCharacter* UnitCharacter = Cast<APCBaseUnitCharacter>(GetOwner());
	if (!UnitCharacter)
		return;

	UPCDataAsset_BaseUnitData* UnitData = UnitCharacter->GetUnitDataAsset();
	if (!UnitData)
		return;
	
	ApplyInitBaseStat(UnitCharacter, UnitData);
	GrantStartupAbilities(UnitData);
	GrantSynergyTags(UnitData);
}

void UPCUnitAbilitySystemComponent::ApplyInitBaseStat(const APCBaseUnitCharacter* Unit, const UPCDataAsset_BaseUnitData* UnitData)
{
	// AttributeSet 스탯 변경은 서버에서만 실행
	if (!GetOwner() || !GetOwner()->HasAuthority() || bInitBaseStatsApplied)
		return;
	
	const int32 UnitLevel = Unit->HasLevelSystem() ? Unit->GetUnitLevel() : 1;
	TMap<FGameplayAttribute, float> AttrMap;
	
	UnitData->FillInitStatMap(UnitLevel, AttrMap);

	for (const auto& KV : AttrMap)
	{
		SetNumericAttributeBase(KV.Key, KV.Value);
	}
	SetNumericAttributeBase(UPCHeroUnitAttributeSet::GetManaRegenAttribute(), 5.f);
	bInitBaseStatsApplied = true;
}

void UPCUnitAbilitySystemComponent::GrantStartupAbilities(UPCDataAsset_BaseUnitData* UnitData)
{
	// GA 능력 부여는 서버에서만 실행
	if (!GetOwner() || !GetOwner()->HasAuthority())
		return;
	
	TArray<TSubclassOf<UGameplayAbility>> GrantAbilities;
	UnitData->FillStartupAbilities(GrantAbilities);

	UPCDataAsset_UnitAbilityConfig* UnitAbilityConfig = UnitData->GetAbilityConfigData();
	
	for (const auto& GAClass : GrantAbilities)
	{
		if (!*GAClass || FindAbilitySpecFromClass(GAClass))
			continue;

		GiveAbility(FGameplayAbilitySpec(GAClass, 1, INDEX_NONE, UnitAbilityConfig));
	}
}

void UPCUnitAbilitySystemComponent::GrantSynergyTags(const UPCDataAsset_BaseUnitData* UnitData)
{
	FGameplayTagContainer SynergyTags;
	UnitData->GetSynergyTags(SynergyTags);

	// 유닛에 시너지 태그 부여 (클라 복제 X)
	AddLooseGameplayTags(SynergyTags);
}
