// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Unit/PCHeroStatusHoverPanel.h"

#include "AbilitySystemGlobals.h"
#include "AbilitySystem/Unit/AttributeSet/PCHeroUnitAttributeSet.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UPCHeroStatusHoverPanel::InitFromHero(AActor* InHero)
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

void UPCHeroStatusHoverPanel::ShowPanelForHero(AActor* InHero)
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
	if (!ASC.IsValid())
		return;

	HandleMap.Reset();
	for (const auto& KV : AttrRoute)
	{
		HandleMap.Add(KV.Key,
			ASC->GetGameplayAttributeValueChangeDelegate(KV.Key)
				.AddUObject(this, &ThisClass::OnAttrChanged));
	}
}

void UPCHeroStatusHoverPanel::UnbindAll()
{
	if (ASC.IsValid())
	{
		auto* A = ASC.Get();
		for (const auto& KV : HandleMap)
		{
			A->GetGameplayAttributeValueChangeDelegate(KV.Key).Remove(KV.Value);
		}
	}

	HandleMap.Reset();
	ASC = nullptr;
	Hero = nullptr;
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

void UPCHeroStatusHoverPanel::ApplyAll() const
{
	if (!ASC.IsValid())
		return;

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

	// 스탯들
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