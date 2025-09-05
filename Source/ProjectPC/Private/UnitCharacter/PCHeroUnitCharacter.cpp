// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/UnitCharacter/PCHeroUnitCharacter.h"

#include "BaseGameplayTags.h"
#include "AbilitySystem/Unit/PCHeroUnitAbilitySystemComponent.h"
#include "AbilitySystem/Unit/AttributeSet/PCHeroUnitAttributeSet.h"
#include "Components/WidgetComponent.h"
#include "DataAsset/Unit/PCDataAsset_HeroUnitData.h"
#include "Net/UnrealNetwork.h"
#include "UI/Unit/PCHeroStatusBarWidget.h"


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

FGameplayTag APCHeroUnitCharacter::GetUnitTypeTag() const
{
	return UnitGameplayTags::Unit_Type_Hero;
}

void APCHeroUnitCharacter::LevelUp()
{
	// LevelUp은 서버권한
	if (!HasAuthority() || !HeroUnitAbilitySystemComponent)
		return;

	HeroLevel = FMath::Clamp(++HeroLevel, 1, 3);
	HeroUnitAbilitySystemComponent->UpdateGAS();
	// Listen Server인 경우 OnRep 수동 호출 (Listen Server 환경 대응, OnRep_HeroLevel 이벤트 못받기 때문)
	if (GetNetMode() == NM_ListenServer)
		OnRep_HeroLevel();
}

void APCHeroUnitCharacter::UpdateStatusBarUI() const
{
	if (UPCHeroStatusBarWidget* StatusBar = Cast<UPCHeroStatusBarWidget>(StatusBarComp->GetUserWidgetObject()))
	{
		StatusBar->SetVariantByLevel(HeroLevel);
	}
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

void APCHeroUnitCharacter::OnRep_HeroLevel() const
{
	// 클라에서 플레이어에게 보여주는 로직 ex)레벨업 이펙트, Status Bar UI 체인지
	UpdateStatusBarUI();
}

void APCHeroUnitCharacter::SetUnitLevel(const int32 Level)
{
	// 레벨 데이터 직접적인 수정은 서버권한
	if (!HasAuthority() || !HeroUnitAbilitySystemComponent)
		return;
	
	HeroLevel = FMath::Clamp(Level, 1, 3);
	HeroUnitAbilitySystemComponent->UpdateGAS();
	// Listen Server인 경우 OnRep 수동 호출 (Listen Server 환경 대응, OnRep_HeroLevel 이벤트 못받기 때문)
	if (GetNetMode() == NM_ListenServer)
		OnRep_HeroLevel();
}

void APCHeroUnitCharacter::SetUnitDataAsset(UPCDataAsset_BaseUnitData* InUnitDataAsset)
{
	HeroUnitDataAsset = Cast<UPCDataAsset_HeroUnitData>(InUnitDataAsset);
}

void APCHeroUnitCharacter::InitStatusBarWidget(UUserWidget* StatusBarWidget)
{
	// 데디서버거나 StatusBar Class가 없으면 실행하지 않음, HasAuthority() 안쓰는 이유: Listen Server 환경 고려
	if (GetNetMode() == NM_DedicatedServer || !StatusBarClass)
		return;

	if (UPCHeroStatusBarWidget* StatusBar = Cast<UPCHeroStatusBarWidget>(StatusBarWidget))
	{
		StatusBar->InitWithASC(GetUnitAbilitySystemComponent(),
			UPCHeroUnitAttributeSet::GetCurrentHealthAttribute(),
			UPCHeroUnitAttributeSet::GetMaxHealthAttribute(),
			UPCHeroUnitAttributeSet::GetCurrentManaAttribute(),
			UPCHeroUnitAttributeSet::GetMaxManaAttribute(),
			HeroLevel);
	}
	
}