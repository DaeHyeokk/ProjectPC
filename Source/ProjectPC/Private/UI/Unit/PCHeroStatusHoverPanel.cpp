// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Unit/PCHeroStatusHoverPanel.h"

#include "AbilitySystem/Unit/PCHeroUnitAbilitySystemComponent.h"
#include "AbilitySystem/Unit/AttributeSet/PCHeroUnitAttributeSet.h"
#include "Character/Unit/PCHeroUnitCharacter.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"


void UPCHeroStatusHoverPanel::InitFromHero(APCHeroUnitCharacter* InHero)
{
	if (!InHero)
	{
		UnbindAll();
		SetVisibility(ESlateVisibility::Collapsed);
		return;
	}
	
	if (Hero.IsValid() && Hero.Get() == InHero)
		return;
	
	UnbindAll();

	Hero = InHero;
	if (UPCHeroUnitAbilitySystemComponent* InHeroASC = Hero->GetHeroUnitAbilitySystemComponent())
	{
		HeroASC = InHeroASC;

		BindAll();
		ApplyAll();
	}
	else
	{
		SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UPCHeroStatusHoverPanel::ShowPanelForHero(APCHeroUnitCharacter* InHero)
{
	InitFromHero(InHero);
	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
}

void UPCHeroStatusHoverPanel::HidePanel()
{
	UnbindAll();
	SetVisibility(ESlateVisibility::Collapsed);
}

void UPCHeroStatusHoverPanel::NativeConstruct()
{
	Super::NativeConstruct();
	BuildRoutes();
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
	
	AttrRoute.Add(UPCHeroUnitAttributeSet::GetBaseDamageAttribute(), EHeroHoverStat::AD);
	AttrRoute.Add(UPCHeroUnitAttributeSet::GetAttackRangeAttribute(), EHeroHoverStat::Range);
	AttrRoute.Add(UPCHeroUnitAttributeSet::GetAttackSpeedAttribute(), EHeroHoverStat::AS);
	AttrRoute.Add(UPCHeroUnitAttributeSet::GetPhysicalDefenseAttribute(), EHeroHoverStat::PDef);
	AttrRoute.Add(UPCHeroUnitAttributeSet::GetMagicDefenseAttribute(), EHeroHoverStat::MDef);
	
	AttrRoute.Add(UPCHeroUnitAttributeSet::GetPhysicalDamageMultiplierAttribute(), EHeroHoverStat::PMul);
	AttrRoute.Add(UPCHeroUnitAttributeSet::GetMagicDamageMultiplierAttribute(), EHeroHoverStat::MMul);
	AttrRoute.Add(UPCHeroUnitAttributeSet::GetCritChanceAttribute(), EHeroHoverStat::CritChance);
	AttrRoute.Add(UPCHeroUnitAttributeSet::GetLifeStealAttribute(), EHeroHoverStat::LifeSteal);
	AttrRoute.Add(UPCHeroUnitAttributeSet::GetSpellVampAttribute(), EHeroHoverStat::SpellVamp);
}

void UPCHeroStatusHoverPanel::BindAll()
{
	if (!HeroASC.IsValid())
		return;

	HandleMap.Reset();
	for (const auto& KV : AttrRoute)
	{
		HandleMap.Add(KV.Key,
			HeroASC->GetGameplayAttributeValueChangeDelegate(KV.Key)
				.AddUObject(this, &ThisClass::OnAttrChanged));
	}
}

void UPCHeroStatusHoverPanel::UnbindAll()
{
	if (HeroASC.IsValid())
	{
		auto* A = HeroASC.Get();
		for (const auto& KV : HandleMap)
		{
			A->GetGameplayAttributeValueChangeDelegate(KV.Key).Remove(KV.Value);
		}
	}

	HandleMap.Reset();
	HeroASC = nullptr;
	Hero = nullptr;
}

void UPCHeroStatusHoverPanel::OnAttrChanged(const FOnAttributeChangeData& Data)
{
	if (!HeroASC.IsValid())
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

		case EHeroHoverStat::AD:
			if (BaseDamageText)
				UpdateText_Int(BaseDamageText, UPCHeroUnitAttributeSet::GetBaseDamageAttribute());
			break;

		case EHeroHoverStat::Range:
			if (AttackRangeText)
				UpdateText_Int(AttackRangeText, UPCHeroUnitAttributeSet::GetAttackRangeAttribute());
			break;

		case EHeroHoverStat::AS:
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
				UpdateText_PctMul(PMultiplierText, UPCHeroUnitAttributeSet::GetPhysicalDamageMultiplierAttribute());
			break;

		case EHeroHoverStat::MMul:
			if (MMultiplierText)
				UpdateText_PctMul(MMultiplierText, UPCHeroUnitAttributeSet::GetMagicDamageMultiplierAttribute());
			break;

		case EHeroHoverStat::CritChance:
			if (CritChanceText)
				UpdateText_Pct01(CritChanceText, UPCHeroUnitAttributeSet::GetCritChanceAttribute());
			break;

		case EHeroHoverStat::LifeSteal:
			if (LifeStealText)
				UpdateText_Pct01(LifeStealText, UPCHeroUnitAttributeSet::GetLifeStealAttribute());
			break;

		case EHeroHoverStat::SpellVamp:
			if (SpellVampText)
				UpdateText_Pct01(SpellVampText, UPCHeroUnitAttributeSet::GetSpellVampAttribute());
			break;

		default:
			break;
		}
	}
}

void UPCHeroStatusHoverPanel::UpdateHP() const
{
	if (!HeroASC.IsValid()) return;
	const float MaxHP = FMath::Max(1.f, HeroASC->GetNumericAttribute(UPCHeroUnitAttributeSet::GetMaxHealthAttribute()));
	const float CurHP = HeroASC->GetNumericAttribute(UPCHeroUnitAttributeSet::GetCurrentHealthAttribute());
	if (HealthBar) HealthBar->SetPercent(CurHP / MaxHP);
	if (HealthText) HealthText->SetText(FText::FromString(FString::Printf(TEXT("%.0f / %.0f"), CurHP, MaxHP)));
}

void UPCHeroStatusHoverPanel::UpdateMP() const
{
	if (!HeroASC.IsValid()) return;
	const float MaxMP = FMath::Max(0.f, HeroASC->GetNumericAttribute(UPCHeroUnitAttributeSet::GetMaxManaAttribute()));
	const float CurMP = HeroASC->GetNumericAttribute(UPCHeroUnitAttributeSet::GetCurrentManaAttribute());
	if (ManaBar) ManaBar->SetPercent(MaxMP > 0.f ? CurMP / MaxMP : 0.f);
	if (ManaText) ManaText->SetText(FText::FromString(FString::Printf(TEXT("%.0f / %.0f"), CurMP, MaxMP)));
}

void UPCHeroStatusHoverPanel::UpdateText_Int(UTextBlock* TextBlock, const FGameplayAttribute& Attr) const
{
	if (!TextBlock || !HeroASC.IsValid()) return;
	TextBlock->SetText(AsInt(HeroASC->GetNumericAttribute(Attr)));
}

void UPCHeroStatusHoverPanel::UpdateText_F2(UTextBlock* TextBlock, const FGameplayAttribute& Attr) const
{
	if (!TextBlock || !HeroASC.IsValid()) return;
	TextBlock->SetText(AsFloat2(HeroASC->GetNumericAttribute(Attr)));
}

void UPCHeroStatusHoverPanel::UpdateText_Pct01(UTextBlock* TextBlock, const FGameplayAttribute& Attr) const
{
	if (!TextBlock || !HeroASC.IsValid()) return;
	TextBlock->SetText(AsPercent0_From01(HeroASC->GetNumericAttribute(Attr)));
}

void UPCHeroStatusHoverPanel::UpdateText_PctMul(UTextBlock* TextBlock, const FGameplayAttribute& Attr) const
{
	if (!TextBlock || !HeroASC.IsValid()) return;
	TextBlock->SetText(AsPercent0_FromMul(HeroASC->GetNumericAttribute(Attr)));
}

void UPCHeroStatusHoverPanel::ApplyAll() const
{
	if (!HeroASC.IsValid())
		return;

	const auto MaxHPAttr = UPCHeroUnitAttributeSet::GetMaxHealthAttribute();
	const auto CurHPAttr = UPCHeroUnitAttributeSet::GetCurrentHealthAttribute();
	const auto MaxMPAttr = UPCHeroUnitAttributeSet::GetMaxManaAttribute();
	const auto CurMPAttr = UPCHeroUnitAttributeSet::GetCurrentManaAttribute();

	const float MaxHP = FMath::Max(1.f, HeroASC->GetNumericAttribute(MaxHPAttr));
	const float CurHP = HeroASC->GetNumericAttribute(CurHPAttr);
	const float MaxMP = FMath::Max(0.f, HeroASC->GetNumericAttribute(MaxMPAttr));
	const float CurMP = HeroASC->GetNumericAttribute(CurMPAttr);

	if (HealthBar)
		HealthBar->SetPercent(CurHP / MaxHP);
	if (ManaBar)
		ManaBar->SetPercent(MaxMP > 0.f ? CurMP / MaxMP : 0.f);
	if (HealthText)
		HealthText->SetText(FText::FromString(FString::Printf(TEXT("%.0f / %.0f"), CurHP, MaxHP)));
	if (ManaText)
		ManaText->SetText(FText::FromString(FString::Printf(TEXT("%.0f / %.0f"), CurMP, MaxMP)));

	// 스탯들
	if (AttackRangeText)
		AttackRangeText->SetText(AsInt(HeroASC->GetNumericAttribute(UPCHeroUnitAttributeSet::GetAttackRangeAttribute())));
	if (BaseDamageText)
		BaseDamageText->SetText(AsInt(HeroASC->GetNumericAttribute(UPCHeroUnitAttributeSet::GetBaseDamageAttribute())));
	if (AttackSpeedText)
		AttackSpeedText->SetText(AsFloat2(HeroASC->GetNumericAttribute(UPCHeroUnitAttributeSet::GetAttackSpeedAttribute())));
	if (PDefText)
		PDefText->SetText(AsInt(HeroASC->GetNumericAttribute(UPCHeroUnitAttributeSet::GetPhysicalDefenseAttribute())));
	if (MDefText)
		MDefText->SetText(AsInt(HeroASC->GetNumericAttribute(UPCHeroUnitAttributeSet::GetMagicDefenseAttribute())));

	// 배수/확률 → % 표시
	if (PMultiplierText)
		PMultiplierText->SetText(AsPercent0_FromMul(HeroASC->GetNumericAttribute(UPCHeroUnitAttributeSet::GetPhysicalDamageMultiplierAttribute())));
	if (MMultiplierText)
		MMultiplierText->SetText(AsPercent0_FromMul(HeroASC->GetNumericAttribute(UPCHeroUnitAttributeSet::GetMagicDamageMultiplierAttribute())));
	if (CritChanceText)
		CritChanceText->SetText(AsPercent0_From01(HeroASC->GetNumericAttribute(UPCHeroUnitAttributeSet::GetCritChanceAttribute())));
	if (LifeStealText)
		LifeStealText->SetText(AsPercent0_From01(HeroASC->GetNumericAttribute(UPCHeroUnitAttributeSet::GetLifeStealAttribute())));
	if (SpellVampText)
		SpellVampText->SetText(AsPercent0_From01(HeroASC->GetNumericAttribute(UPCHeroUnitAttributeSet::GetSpellVampAttribute())));
}