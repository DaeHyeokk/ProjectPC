// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Unit/PCHeroUnitCharacter.h"

#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"

#include "AbilitySystem/Unit/PCHeroUnitAbilitySystemComponent.h"
#include "AbilitySystem/Unit/AttributeSet/PCHeroUnitAttributeSet.h"
#include "BaseGameplayTags.h"
#include "Controller/Unit/PCUnitAIController.h"
#include "DataAsset/Unit/PCDataAsset_HeroUnitData.h"
#include "GameFramework/GameState/PCCombatGameState.h"
#include "UI/Unit/PCHeroStatusBarWidget.h"
#include "UI/Unit/PCUnitStatusBarWidget.h"


APCHeroUnitCharacter::APCHeroUnitCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	
	HeroUnitAbilitySystemComponent = CreateDefaultSubobject<UPCHeroUnitAbilitySystemComponent>(TEXT("HeroUnitAbilitySystemComponent"));

	if (HeroUnitAbilitySystemComponent)
	{
		HeroUnitAbilitySystemComponent->SetIsReplicated(true);
		HeroUnitAbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

		UPCHeroUnitAttributeSet* HeroAttrSet = CreateDefaultSubobject<UPCHeroUnitAttributeSet>(TEXT("HeroUnitAttributeSet"));
		HeroUnitAbilitySystemComponent->AddAttributeSetSubobject(HeroAttrSet);
		HeroUnitAttributeSet = HeroUnitAbilitySystemComponent->GetSet<UPCHeroUnitAttributeSet>();
	}
}

void APCHeroUnitCharacter::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(APCHeroUnitCharacter, HeroLevel);
}

UPCHeroUnitAbilitySystemComponent* APCHeroUnitCharacter::GetHeroUnitAbilitySystemComponent() const
{
	return HeroUnitAbilitySystemComponent;
}

const UPCHeroUnitAttributeSet* APCHeroUnitCharacter::GetHeroUnitAttributeSet()
{
	if (!HeroUnitAttributeSet)
	{
		if (const UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
		{
			HeroUnitAttributeSet = ASC->GetSet<UPCHeroUnitAttributeSet>();
		}
	}
	return HeroUnitAttributeSet;
}

UPCUnitAbilitySystemComponent* APCHeroUnitCharacter::GetUnitAbilitySystemComponent() const
{
	return HeroUnitAbilitySystemComponent;
}

void APCHeroUnitCharacter::LevelUp()
{
	// LevelUp은 서버권한
	if (!HasAuthority() || !HeroUnitAbilitySystemComponent)
		return;

	FGameplayCueParameters Params;
	Params.TargetAttachComponent = GetMesh();
	HeroUnitAbilitySystemComponent->ExecuteGameplayCue(GameplayCueTags::GameplayCue_VFX_Unit_LevelUp, Params);
	
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

void APCHeroUnitCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if (auto* ASC = GetAbilitySystemComponent())
	{
		SynergyTagChangedHandle = ASC->RegisterGameplayTagEvent(SynergyGameplayTags::Synergy)
		.AddUObject(this, &ThisClass::OnSynergyTagChanged);
	}
}

void APCHeroUnitCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	OnHeroDestroyed.Broadcast(this);
	
	if (SynergyTagChangedHandle.IsValid())
		GetAbilitySystemComponent()->RegisterGameplayTagEvent(SynergyGameplayTags::Synergy).Remove(SynergyTagChangedHandle);
	
	Super::EndPlay(EndPlayReason);
	
}

void APCHeroUnitCharacter::RestoreFromCombatEnd()
{
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::Type::QueryAndPhysics);
	
	if (HasAuthority())
	{
		if (!HeroUnitAbilitySystemComponent || !HeroUnitAttributeSet)
			return;

		// Max Health 값으로 Current Health 초기화
		HeroUnitAbilitySystemComponent->ApplyModToAttribute(
			UPCUnitAttributeSet::GetCurrentHealthAttribute(),
			EGameplayModOp::Override,
			HeroUnitAttributeSet->GetMaxHealth());

		if (!HeroUnitDataAsset)
			return;

		// 기본으로 가지는 Current Mana로 초기화
		HeroUnitAbilitySystemComponent->ApplyModToAttribute(
			UPCHeroUnitAttributeSet::GetCurrentManaAttribute(),
			EGameplayModOp::Override,
			HeroUnitDataAsset->GetCombatStartMana());

		// 이전 전투에서 사망했을 경우 사망 태그 제거
		if (HeroUnitAbilitySystemComponent->HasMatchingGameplayTag(UnitGameplayTags::Unit_State_Combat_Dead))
		{
			HeroUnitAbilitySystemComponent->RemoveLooseGameplayTag(UnitGameplayTags::Unit_State_Combat_Dead);
			HeroUnitAbilitySystemComponent->RemoveReplicatedLooseGameplayTag(UnitGameplayTags::Unit_State_Combat_Dead);
			//bIsDead = false;
		}

		// 스턴 상태일 경우 스턴 태그 제거
		if (HeroUnitAbilitySystemComponent->HasMatchingGameplayTag(UnitGameplayTags::Unit_State_Combat_Stun))
		{
			HeroUnitAbilitySystemComponent->RemoveActiveEffectsWithGrantedTags(
				FGameplayTagContainer(UnitGameplayTags::Unit_State_Combat_Stun));
			//bIsStunned = false;
		}
		
		HeroUnitAbilitySystemComponent->CurrentMontageStop(0.2f);
		
		// 블랙보드 키값 초기화
		if (APCUnitAIController* AIC = Cast<APCUnitAIController>(GetController()))
		{
			AIC->ClearBlackboardValue();
		}
	}
}

void APCHeroUnitCharacter::ActionDrag(const bool IsStart)
{
	// 클라에서만 실행 (Listen Server 포함)
	if (GetNetMode() == NM_DedicatedServer)
		return;
	
	if (GetMesh())
	{
		//GetMesh()->SetVisibility(!IsStart, true);
	}
}

void APCHeroUnitCharacter::OnSynergyTagChanged(const FGameplayTag Tag, int32 NewCount) const
{
	// 벤치에 있는 경우 해당 이벤트 무시
	if (!bIsOnField)
		return;
	
	if (Tag.MatchesTag(SynergyGameplayTags::Synergy))
	{
		if (NewCount >= 1)
		{
			OnHeroSynergyTagChanged.Broadcast(this, Tag, true);
		}
		else
		{
			OnHeroSynergyTagChanged.Broadcast(this, Tag, false);
		}
	}
}

void APCHeroUnitCharacter::OnRep_HeroLevel()
{
	// 클라에서 플레이어에게 보여주는 로직 ex) Status Bar UI 체인지
	UpdateStatusBarUI();
}

void APCHeroUnitCharacter::ChangedOnTile(const bool IsOnField)
{
	Super::ChangedOnTile(IsOnField);
}

void APCHeroUnitCharacter::OnGameStateChanged(const FGameplayTag& NewStateTag)
{
	const FGameplayTag& CombatPreparationTag = GameStateTags::Game_State_Combat_Preparation;
	const FGameplayTag& CombatActiveTag = GameStateTags::Game_State_Combat_Active;
	const FGameplayTag& CombatEndTag = GameStateTags::Game_State_Combat_End;

	if (NewStateTag == CombatActiveTag)
	{
		
	}
	else if (NewStateTag == CombatEndTag)
	{
		RestoreFromCombatEnd();
	}
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
