// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Unit/PCCreepUnitCharacter.h"

#include "BaseGameplayTags.h"
#include "AbilitySystem/Unit/PCUnitAbilitySystemComponent.h"
#include "AbilitySystem/Unit/AttributeSet/PCUnitAttributeSet.h"
#include "Blueprint/UserWidget.h"
#include "DataAsset/Unit/PCDataAsset_CreepUnitData.h"
#include "GameFramework/PlayerState/PCPlayerState.h"
#include "UI/Unit/PCUnitStatusBarWidget.h"


APCCreepUnitCharacter::APCCreepUnitCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	UnitAbilitySystemComponent = CreateDefaultSubobject<UPCUnitAbilitySystemComponent>(TEXT("UnitAbilitySystemComponent"));

	if (UnitAbilitySystemComponent)
	{
		UnitAbilitySystemComponent->SetIsReplicated(true);
		UnitAbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

		UPCUnitAttributeSet* UnitAttrSet = CreateDefaultSubobject<UPCUnitAttributeSet>(TEXT("HeroUnitAttributeSet"));
		UnitAbilitySystemComponent->AddAttributeSetSubobject(UnitAttrSet);
		UnitAttributeSet = UnitAbilitySystemComponent->GetSet<UPCUnitAttributeSet>();
	}
}

UPCUnitAbilitySystemComponent* APCCreepUnitCharacter::GetUnitAbilitySystemComponent() const
{
	return UnitAbilitySystemComponent;
}

void APCCreepUnitCharacter::SetUnitDataAsset(UPCDataAsset_BaseUnitData* InUnitDataAsset)
{
	CreepUnitDataAsset = Cast<UPCDataAsset_CreepUnitData>(InUnitDataAsset);
}

void APCCreepUnitCharacter::InitStatusBarWidget(UUserWidget* StatusBarWidget)
{
	// 데디서버거나 StatusBar Class가 없으면 실행하지 않음, HasAuthority() 안쓰는 이유: Listen Server 환경 고려
	if (GetNetMode() == NM_DedicatedServer || !StatusBarClass)
		return;

	if (UPCUnitStatusBarWidget* StatusBar = Cast<UPCUnitStatusBarWidget>(StatusBarWidget))
	{
		StatusBar->InitWithASC(this, GetAbilitySystemComponent(),
			UPCUnitAttributeSet::GetCurrentHealthAttribute(),
			UPCUnitAttributeSet::GetMaxHealthAttribute()
			);
	}
}

void APCCreepUnitCharacter::OnGameStateChanged(const FGameplayTag& NewStateTag)
{
	const FGameplayTag& CombatPreparationTag = GameStateTags::Game_State_Combat_Preparation;
	const FGameplayTag& CombatActiveTag = GameStateTags::Game_State_Combat_Active;
	const FGameplayTag& CombatEndTag = GameStateTags::Game_State_Combat_End;

	if (NewStateTag == CombatActiveTag)
	{
		
	}
	else if (NewStateTag == CombatEndTag)
	{
		if (HasAuthority())
		{
			Destroy();
		}
	}
}

void APCCreepUnitCharacter::Die()
{
	if (OnCombatBoard)
	{
		
	}
	
	Super::Die();
}
