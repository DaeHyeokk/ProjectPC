// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Unit/PCHeroUnitAbilitySystemComponent.h"

#include "BaseGameplayTags.h"
#include "Character/Unit/PCHeroUnitCharacter.h"
#include "DataAsset/Unit/PCDataAsset_HeroUnitData.h"

void UPCHeroUnitAbilitySystemComponent::UpdateGAS()
{
	UpdateBaseStatForLevel();
	UpdateUltimateAbilityForLevel();
}

void UPCHeroUnitAbilitySystemComponent::GrantStartupAbilities()
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
		return;
	
	const APCBaseUnitCharacter* UnitCharacter = Cast<APCBaseUnitCharacter>(GetOwner());
	if (!UnitCharacter)
		return;
	
	const UPCDataAsset_BaseUnitData* UnitData = UnitCharacter->GetUnitDataAsset();
	if (!UnitData)
		return;
	
	Super::GrantStartupAbilities();

	if (const UPCDataAsset_HeroUnitData* HeroUnitData = Cast<UPCDataAsset_HeroUnitData>(UnitData))
	{
		TArray<TSubclassOf<UGameplayAbility>> GrantUltAbilities;
		HeroUnitData->FillStartupUltimateAbilities(GrantUltAbilities);

		for (const auto& GAClass : GrantUltAbilities)
		{
			if (!*GAClass || FindAbilitySpecFromClass(GAClass))
				continue;

			GiveAbility(FGameplayAbilitySpec(GAClass, 1, INDEX_NONE, GetOwner()));
		}
	}
}

void UPCHeroUnitAbilitySystemComponent::UpdateBaseStatForLevel()
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
		return;
	
	const APCHeroUnitCharacter* HeroCharacter = Cast<APCHeroUnitCharacter>(GetOwner());
	if (!HeroCharacter)
		return;
	
	const UPCDataAsset_BaseUnitData* UnitData = HeroCharacter->GetUnitDataAsset();
	if (!UnitData)
		return;
	
	TMap<FGameplayAttribute, float> StatMap;
	UnitData->FillInitStatMap(HeroCharacter->GetUnitLevel(), StatMap);
	if (StatMap.Num() == 0)
		return;
	
	for (const auto& KV : StatMap)
	{
		SetNumericAttributeBase(KV.Key, KV.Value);
	}
}

void UPCHeroUnitAbilitySystemComponent::UpdateUltimateAbilityForLevel()
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
		return;

	const APCHeroUnitCharacter* HeroCharacter = Cast<APCHeroUnitCharacter>(GetOwner());
	if (!HeroCharacter)
		return;
	
	FGameplayTagContainer UltimateTagContainer;
	UltimateTagContainer.AddTag(UnitGameplayTags::Unit_Action_Attack_Ultimate);
	
	TArray<FGameplayAbilitySpecHandle> UltHandles;
	FindAllAbilitiesWithTags(UltHandles, UltimateTagContainer, false);
	
	for (const FGameplayAbilitySpecHandle& SpecHandle : UltHandles)
	{
		if (FGameplayAbilitySpec* Spec = FindAbilitySpecFromHandle(SpecHandle))
		{
			Spec->Level = HeroCharacter->GetUnitLevel();
			MarkAbilitySpecDirty(*Spec);
		}
	}
}