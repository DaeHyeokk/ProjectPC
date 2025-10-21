// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Unit/PCHeroStatusHoverPanel.h"

#include "AbilitySystemGlobals.h"
#include "AbilitySystem/Unit/AttributeSet/PCHeroUnitAttributeSet.h"
#include "Character/Unit/PCCommonUnitCharacter.h"
#include "Character/Unit/PCHeroUnitCharacter.h"
#include "Component/PCUnitEquipmentComponent.h"
#include "Components/HorizontalBox.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Engine/AssetManager.h"
#include "GameFramework/GameState/PCCombatGameState.h"
#include "Shop/PCShopManager.h"
#include "UI/Item/PCItemSlotWidget.h"
#include "UI/Shop/PCUnitSlotWidget.h"

void UPCHeroStatusHoverPanel::InitFromHero(APCCommonUnitCharacter* InHero)
{
	if (!InHero)
	{
		UnbindAll();
		SetVisibility(ESlateVisibility::Collapsed);
		return;
	}
	
	if (CurHero.IsValid() && CurHero.Get() == InHero)
		return;
	
	UnbindAll();

	CurHero = InHero;
	ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(InHero);
	
	if (ASC.IsValid())
	{
		BindAll();
		ApplyAll();
	}
	else
	{
		SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UPCHeroStatusHoverPanel::ShowPanelForHero(APCCommonUnitCharacter* InHero)
{
	InitFromHero(InHero);
	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
}

void UPCHeroStatusHoverPanel::HidePanel()
{
	UnbindAll();
	SetVisibility(ESlateVisibility::Collapsed);
}

void UPCHeroStatusHoverPanel::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	BuildRoutes();

	if (ItemSlotPanel)
	{
		TArray<UWidget*> ItemSlots = ItemSlotPanel->GetAllChildren();
		for (UWidget* Widget : ItemSlots)
		{
			if (UPCItemSlotWidget* ItemSlot = Cast<UPCItemSlotWidget>(Widget))
				ItemSlotWidgets.Add(ItemSlot);
		}
	}
}

void UPCHeroStatusHoverPanel::NativeDestruct()
{
	UnbindAll();
	Super::NativeDestruct();
}

void UPCHeroStatusHoverPanel::BuildRoutes()
{
	AttrRoute.Reset();

	AttrRoute.Add(UPCHeroUnitAttributeSet::GetMaxHealthAttribute(), EHeroHoverStat::MaxHP);
	AttrRoute.Add(UPCHeroUnitAttributeSet::GetCurrentHealthAttribute(), EHeroHoverStat::CurHP);
	AttrRoute.Add(UPCHeroUnitAttributeSet::GetMaxManaAttribute(), EHeroHoverStat::MaxMP);
	AttrRoute.Add(UPCHeroUnitAttributeSet::GetCurrentManaAttribute(), EHeroHoverStat::CurMP);
	AttrRoute.Add(UPCHeroUnitAttributeSet::GetManaRegenAttribute(), EHeroHoverStat::ManaRegen);
	
	AttrRoute.Add(UPCHeroUnitAttributeSet::GetBaseDamageAttribute(), EHeroHoverStat::AD);
	AttrRoute.Add(UPCHeroUnitAttributeSet::GetAttackRangeAttribute(), EHeroHoverStat::Range);
	AttrRoute.Add(UPCHeroUnitAttributeSet::GetAttackSpeedAttribute(), EHeroHoverStat::AS);
	AttrRoute.Add(UPCHeroUnitAttributeSet::GetAttackSpeedIncreaseMultiplierAttribute(), EHeroHoverStat::ASInc);
	AttrRoute.Add(UPCHeroUnitAttributeSet::GetAttackSpeedDecreaseMultiplierAttribute(), EHeroHoverStat::ASDec);
	AttrRoute.Add(UPCHeroUnitAttributeSet::GetPhysicalDefenseAttribute(), EHeroHoverStat::PDef);
	AttrRoute.Add(UPCHeroUnitAttributeSet::GetMagicDefenseAttribute(), EHeroHoverStat::MDef);
	
	AttrRoute.Add(UPCHeroUnitAttributeSet::GetPhysicalDamageMultiplierAttribute(), EHeroHoverStat::PMul);
	AttrRoute.Add(UPCHeroUnitAttributeSet::GetMagicDamageMultiplierAttribute(), EHeroHoverStat::MMul);
	AttrRoute.Add(UPCHeroUnitAttributeSet::GetDamageMultiplierAttribute(), EHeroHoverStat::DamageMul);
	AttrRoute.Add(UPCHeroUnitAttributeSet::GetCritChanceAttribute(), EHeroHoverStat::CritChance);
	AttrRoute.Add(UPCHeroUnitAttributeSet::GetLifeStealAttribute(), EHeroHoverStat::LifeSteal);
}

void UPCHeroStatusHoverPanel::BindAll()
{
	if (!CurHero.IsValid() || !ASC.IsValid())
		return;

	AttrChangedHandleMap.Reset();
	for (const auto& KV : AttrRoute)
	{
		AttrChangedHandleMap.Add(KV.Key,
			ASC->GetGameplayAttributeValueChangeDelegate(KV.Key)
				.AddUObject(this, &ThisClass::OnAttrChanged));
	}

	if (APCHeroUnitCharacter* Hero = Cast<APCHeroUnitCharacter>(CurHero.Get()))
	{
		if (UPCUnitEquipmentComponent* EquipmentComp = Hero->GetEquipmentComponent())
		{
			EquipItemChangedHandle = EquipmentComp->OnEquipItemChanged
				.AddUObject(this, &ThisClass::OnEquipItemChanged);
		}

		HeroLevelChangedHandle = Hero->OnHeroLevelUp.AddUObject(this, &ThisClass::OnHeroLevelChanged);
	}
}

void UPCHeroStatusHoverPanel::UnbindAll()
{
	if (ASC.IsValid())
	{
		auto* A = ASC.Get();
		for (const auto& KV : AttrChangedHandleMap)
		{
			A->GetGameplayAttributeValueChangeDelegate(KV.Key).Remove(KV.Value);
		}
	}
	
	if (CurHero.IsValid() && EquipItemChangedHandle.IsValid())
	{
		if (APCHeroUnitCharacter* Hero = Cast<APCHeroUnitCharacter>(CurHero.Get()))
		{
			if (UPCUnitEquipmentComponent* EquipmentComp = Hero->GetEquipmentComponent())
			{
				EquipmentComp->OnEquipItemChanged.Remove(EquipItemChangedHandle);
			}

			if (HeroLevelChangedHandle.IsValid())
			{
				Hero->OnHeroLevelUp.Remove(HeroLevelChangedHandle);
				HeroLevelChangedHandle.Reset();
			}
		}
		EquipItemChangedHandle.Reset();
	}
	
	AttrChangedHandleMap.Reset();
	ASC = nullptr;
	CurHero = nullptr;
}

void UPCHeroStatusHoverPanel::OnAttrChanged(const FOnAttributeChangeData& Data)
{
	if (!ASC.IsValid())
		return;

	if (const EHeroHoverStat* ChangeAttr = AttrRoute.Find(Data.Attribute))
	{
		switch (*ChangeAttr)
		{
		case EHeroHoverStat::MaxHP:
		case EHeroHoverStat::CurHP:
			UpdateHP();
			break;

		case EHeroHoverStat::MaxMP:
		case EHeroHoverStat::CurMP:
			UpdateMP();
			break;

		case EHeroHoverStat::ManaRegen:
			if (ManaRegenText)
				UpdateText_RegenValue(ManaRegenText, UPCHeroUnitAttributeSet::GetManaRegenAttribute());
			break;
			
		case EHeroHoverStat::AD:
			if (BaseDamageText)
				UpdateText_Int(BaseDamageText, UPCHeroUnitAttributeSet::GetBaseDamageAttribute());
			break;

		case EHeroHoverStat::Range:
			if (AttackRangeText)
				UpdateText_Int(AttackRangeText, UPCHeroUnitAttributeSet::GetAttackRangeAttribute());
			break;

		case EHeroHoverStat::AS:
		case EHeroHoverStat::ASInc:
		case EHeroHoverStat::ASDec:
			if (AttackSpeedText)
				UpdateText_F2(AttackSpeedText, UPCHeroUnitAttributeSet::GetAttackSpeedAttribute());
			break;

		case EHeroHoverStat::PDef:
			if (PDefText)
				UpdateText_Int(PDefText, UPCHeroUnitAttributeSet::GetPhysicalDefenseAttribute());
			break;

		case EHeroHoverStat::MDef:
			if (MDefText)
				UpdateText_Int(MDefText, UPCHeroUnitAttributeSet::GetMagicDefenseAttribute());
			break;

		case EHeroHoverStat::PMul:
			if (PMultiplierText)
				UpdateText_PctValue(PMultiplierText, UPCHeroUnitAttributeSet::GetPhysicalDamageMultiplierAttribute());
			break;

		case EHeroHoverStat::MMul:
			if (MMultiplierText)
				UpdateText_PctValue(MMultiplierText, UPCHeroUnitAttributeSet::GetMagicDamageMultiplierAttribute());
			break;

		case EHeroHoverStat::CritChance:
			if (CritChanceText)
				UpdateText_PctValue(CritChanceText, UPCHeroUnitAttributeSet::GetCritChanceAttribute());
			break;

		case EHeroHoverStat::LifeSteal:
			if (LifeStealText)
				UpdateText_PctValue(LifeStealText, UPCHeroUnitAttributeSet::GetLifeStealAttribute());
			break;

		case EHeroHoverStat::DamageMul:
			if (DamageMultiplierText)
				UpdateText_PctValue(DamageMultiplierText, UPCHeroUnitAttributeSet::GetDamageMultiplierAttribute());
			break;
			
		default:
			break;
		}
	}
}

void UPCHeroStatusHoverPanel::OnEquipItemChanged() const
{
	UpdateEquipItemSlots();
}

void UPCHeroStatusHoverPanel::OnHeroLevelChanged() const
{
	UpdateLevel();
}

void UPCHeroStatusHoverPanel::UpdateHP() const
{
	if (!ASC.IsValid()) return;
	const float MaxHP = FMath::Max(1.f, ASC->GetNumericAttribute(UPCHeroUnitAttributeSet::GetMaxHealthAttribute()));
	const float CurHP = ASC->GetNumericAttribute(UPCHeroUnitAttributeSet::GetCurrentHealthAttribute());
	if (HealthBar) HealthBar->SetPercent(CurHP / MaxHP);
	if (HealthText) HealthText->SetText(FText::FromString(FString::Printf(TEXT("%.0f / %.0f"), CurHP, MaxHP)));
}

void UPCHeroStatusHoverPanel::UpdateMP() const
{
	if (!ASC.IsValid()) return;
	const float MaxMP = FMath::Max(0.f, ASC->GetNumericAttribute(UPCHeroUnitAttributeSet::GetMaxManaAttribute()));
	const float CurMP = ASC->GetNumericAttribute(UPCHeroUnitAttributeSet::GetCurrentManaAttribute());
	if (ManaBar) ManaBar->SetPercent(MaxMP > 0.f ? CurMP / MaxMP : 0.f);
	if (ManaText) ManaText->SetText(FText::FromString(FString::Printf(TEXT("%.0f / %.0f"), CurMP, MaxMP)));
}

void UPCHeroStatusHoverPanel::UpdateEquipItemSlots() const
{
	TArray<FGameplayTag> ItemTags = CurHero->GetEquipItemTags();
	for (int32 i=0; i<ItemSlotWidgets.Num(); ++i)
	{
		if (ItemTags.IsValidIndex(i))
		{
			FGameplayTag ItemTag = ItemTags[i];
			if (ItemTag.IsValid())
			{
				ItemSlotWidgets[i]->SetItem(ItemTag);
				continue;
			}
		}

		ItemSlotWidgets[i]->RemoveItem();
	}
}

void UPCHeroStatusHoverPanel::UpdatePosition() const
{
	if (!Img_Position || !PositionText) return;

	if (auto Texture = TextureData.UnitPositionTexture.FindRef(CurHero->GetUnitRecommendedPosition()))
	{
		Img_Position->SetBrushFromTexture(Texture);
	}

	FText Text;
	switch (CurHero->GetUnitRecommendedPosition())
	{
	case EUnitRecommendedPosition::FrontLine:
		Text = FText::FromString(TEXT("전방"));
		break;
	case EUnitRecommendedPosition::BackLine:
		Text = FText::FromString(TEXT("후방"));
		break;
	default:
		Text = FText::FromString(TEXT("중앙"));
		break;
	}
	
	PositionText->SetText(Text);
}

void UPCHeroStatusHoverPanel::UpdateLevel() const
{
	if (!Img_Level) return;

	if (TextureData.UnitLevelTexture.IsValidIndex(CurHero->GetUnitLevel() - 1))
	{
		if (auto Texture = TextureData.UnitLevelTexture[CurHero->GetUnitLevel() - 1])
		{
			Img_Level->SetBrushFromTexture(Texture);
		}
	}
}

void UPCHeroStatusHoverPanel::UpdateText_Int(UTextBlock* TextBlock, const FGameplayAttribute& Attr) const
{
	if (!TextBlock || !ASC.IsValid()) return;
	TextBlock->SetText(AsInt(ASC->GetNumericAttribute(Attr)));
}

void UPCHeroStatusHoverPanel::UpdateText_F2(UTextBlock* TextBlock, const FGameplayAttribute& Attr) const
{
	if (!TextBlock || !ASC.IsValid()) return;
	
	if (Attr == UPCUnitAttributeSet::GetAttackSpeedAttribute())
	{
		const UPCUnitAttributeSet* UnitAttrSet = ASC->GetSet<UPCUnitAttributeSet>();
		const float AttackSpd = UnitAttrSet ? UnitAttrSet->GetEffectiveAttackSpeed() : 0.f;
		TextBlock->SetText(AsFloat2(AttackSpd));	
	}
	else
	{
		TextBlock->SetText(AsFloat2(ASC->GetNumericAttribute(Attr)));	
	}
}

void UPCHeroStatusHoverPanel::UpdateText_Pct01(UTextBlock* TextBlock, const FGameplayAttribute& Attr) const
{
	if (!TextBlock || !ASC.IsValid()) return;
	TextBlock->SetText(AsPercent0_From01(ASC->GetNumericAttribute(Attr)));
}

void UPCHeroStatusHoverPanel::UpdateText_PctValue(UTextBlock* TextBlock, const FGameplayAttribute& Attr) const
{
	if (!TextBlock || !ASC.IsValid()) return;
	TextBlock->SetText(AsPercent0_FromPctValue(ASC->GetNumericAttribute(Attr)));
}

void UPCHeroStatusHoverPanel::UpdateText_RegenValue(UTextBlock* TextBlock, const FGameplayAttribute& Attr) const
{
	if (!TextBlock || !ASC.IsValid()) return;
	TextBlock->SetText(AsRegenPerSec(ASC->GetNumericAttribute(Attr)));
}

void UPCHeroStatusHoverPanel::ApplyAll() const
{
	if (!CurHero.IsValid() || !ASC.IsValid())
		return;

	if (UnitSlotWidget)
	{
		if (auto GS = GetWorld()->GetGameState<APCCombatGameState>())
		{
			auto UnitData = GS->GetShopManager()->GetShopUnitDataByTag(CurHero->GetUnitTag());
	
			if (UnitData.UnitName != "Dummy")
			{
				UnitSlotWidget->Setup(UnitData, false);
			}
		}
	}
	
	UpdatePosition();
	UpdateLevel();

	const auto MaxHPAttr = UPCHeroUnitAttributeSet::GetMaxHealthAttribute();
	const auto CurHPAttr = UPCHeroUnitAttributeSet::GetCurrentHealthAttribute();
	const auto MaxMPAttr = UPCHeroUnitAttributeSet::GetMaxManaAttribute();
	const auto CurMPAttr = UPCHeroUnitAttributeSet::GetCurrentManaAttribute();

	const float MaxHP = FMath::Max(1.f, ASC->GetNumericAttribute(MaxHPAttr));
	const float CurHP = ASC->GetNumericAttribute(CurHPAttr);
	const float MaxMP = FMath::Max(0.f, ASC->GetNumericAttribute(MaxMPAttr));
	const float CurMP = ASC->GetNumericAttribute(CurMPAttr);

	if (HealthBar)
		HealthBar->SetPercent(CurHP / MaxHP);
	if (ManaBar)
		ManaBar->SetPercent(MaxMP > 0.f ? CurMP / MaxMP : 0.f);
	if (HealthText)
		HealthText->SetText(FText::FromString(FString::Printf(TEXT("%.0f / %.0f"), CurHP, MaxHP)));
	if (ManaText)
		ManaText->SetText(FText::FromString(FString::Printf(TEXT("%.0f / %.0f"), CurMP, MaxMP)));

	// Item List
	UpdateEquipItemSlots();
	
	// 스탯들
	if (ManaRegenText)
		ManaRegenText->SetText(AsRegenPerSec(ASC->GetNumericAttribute(UPCHeroUnitAttributeSet::GetManaRegenAttribute())));
	if (AttackRangeText)
		AttackRangeText->SetText(AsInt(ASC->GetNumericAttribute(UPCHeroUnitAttributeSet::GetAttackRangeAttribute())));
	if (BaseDamageText)
		BaseDamageText->SetText(AsInt(ASC->GetNumericAttribute(UPCHeroUnitAttributeSet::GetBaseDamageAttribute())));
	if (AttackSpeedText)
	{
		const UPCUnitAttributeSet* UnitAttrSet = ASC->GetSet<UPCUnitAttributeSet>();
		const float AttackSpd = UnitAttrSet ? UnitAttrSet->GetEffectiveAttackSpeed() : 0.f;
		AttackSpeedText->SetText(AsFloat2(AttackSpd));
	}
	if (PDefText)
		PDefText->SetText(AsInt(ASC->GetNumericAttribute(UPCHeroUnitAttributeSet::GetPhysicalDefenseAttribute())));
	if (MDefText)
		MDefText->SetText(AsInt(ASC->GetNumericAttribute(UPCHeroUnitAttributeSet::GetMagicDefenseAttribute())));

	// 배수/확률 → % 표시
	if (PMultiplierText)
		PMultiplierText->SetText(AsPercent0_FromPctValue(ASC->GetNumericAttribute(UPCHeroUnitAttributeSet::GetPhysicalDamageMultiplierAttribute())));
	if (MMultiplierText)
		MMultiplierText->SetText(AsPercent0_FromPctValue(ASC->GetNumericAttribute(UPCHeroUnitAttributeSet::GetMagicDamageMultiplierAttribute())));
	if (CritChanceText)
		CritChanceText->SetText(AsPercent0_FromPctValue(ASC->GetNumericAttribute(UPCHeroUnitAttributeSet::GetCritChanceAttribute())));
	if (LifeStealText)
		LifeStealText->SetText(AsPercent0_FromPctValue(ASC->GetNumericAttribute(UPCHeroUnitAttributeSet::GetLifeStealAttribute())));
	if (DamageMultiplierText)
		DamageMultiplierText->SetText(AsPercent0_FromPctValue(ASC->GetNumericAttribute(UPCHeroUnitAttributeSet::GetDamageMultiplierAttribute())));
}
