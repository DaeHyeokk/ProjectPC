// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/UnitCharacter/PCHeroUnitCharacter.h"

#include "BaseGameplayTags.h"
#include "AbilitySystem/Unit/PCHeroUnitAbilitySystemComponent.h"
#include "AbilitySystem/Unit/AttributeSet/PCHeroUnitAttributeSet.h"
#include "DataAsset/Unit/PCDataAsset_HeroUnitData.h"
#include "Net/UnrealNetwork.h"


APCHeroUnitCharacter::APCHeroUnitCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	
	HeroUnitAbilitySystemComponent = CreateDefaultSubobject<UPCHeroUnitAbilitySystemComponent>(TEXT("HeroUnitAbilitySystemComponent"));

	if (HeroUnitAbilitySystemComponent)
	{
		HeroUnitAbilitySystemComponent->SetIsReplicated(true);
		HeroUnitAbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

		UPCHeroUnitAttributeSet* HeroUnitAttributeSet = CreateDefaultSubobject<UPCHeroUnitAttributeSet>(TEXT("HeroUnitAttributeSet"));
		HeroUnitAbilitySystemComponent->AddAttributeSetSubobject(HeroUnitAttributeSet);
	}
}

void APCHeroUnitCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void APCHeroUnitCharacter::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(APCHeroUnitCharacter, HeroLevel);
}

UPCHeroUnitAbilitySystemComponent* APCHeroUnitCharacter::GetHeroUnitAbilitySystemComponent()
{
	return HeroUnitAbilitySystemComponent;
}

UPCUnitAbilitySystemComponent* APCHeroUnitCharacter::GetUnitAbilitySystemComponent() const
{
	return HeroUnitAbilitySystemComponent;
}

const UPCDataAsset_BaseUnitData* APCHeroUnitCharacter::GetUnitDataAsset() const
{
	return HeroUnitDataAsset;
}

FGameplayTag APCHeroUnitCharacter::GetUnitTypeTag() const
{
	return UnitGameplayTags::Unit_Type_Hero;
}

void APCHeroUnitCharacter::LevelUp()
{
	if (!HeroUnitAbilitySystemComponent)
		return;

	HeroLevel = FMath::Clamp(++HeroLevel, 1, 3);
	HeroUnitAbilitySystemComponent->UpdateGAS();
}

FGameplayTag APCHeroUnitCharacter::GetJobSynergyTag() const
{
	if (!HeroUnitDataAsset)
		return FGameplayTag::EmptyTag;

	return HeroUnitDataAsset->GetJobSynergyTag();
}

FGameplayTag APCHeroUnitCharacter::GetSpeciesSynergyTag() const
{
	if (!HeroUnitDataAsset)
		return FGameplayTag::EmptyTag;

	return HeroUnitDataAsset->GetSpeciesSynergyTag();
}

void APCHeroUnitCharacter::OnRep_HeroLevel()
{
	// 클라에서 플레이어에게 보여주는 로직 ex)레벨업 이펙트
}
