// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Unit/PCHeroStatusBarWidget.h"
#include "AbilitySystemComponent.h"
#include "Components/WidgetSwitcher.h"
#include "UI/Unit/PCUnitStatusBarWidget.h"

void UPCHeroStatusBarWidget::InitWithASC(UAbilitySystemComponent* InASC,
	FGameplayAttribute InHealthAttr, FGameplayAttribute InMaxHealthAttr,
	FGameplayAttribute InManaAttr, FGameplayAttribute InMaxManaAttr, int32 InLevel)
{
	ASC = InASC;
	HealthAttr = InHealthAttr;
	MaxHealthAttr = InMaxHealthAttr;
	ManaAttr = InManaAttr;
	MaxManaAttr = InMaxManaAttr;

	SetVariantByLevel(InLevel);

	OnVisibilityChanged.AddDynamic(this, &ThisClass::HandleVisibilityChanged);
}

void UPCHeroStatusBarWidget::SetVariantByLevel(int32 Level)
{
	if (!StatusSwitcher)
		return;

	int32 Index = FMath::Clamp(Level-1, 0, StatusSwitcher->GetChildrenCount() - 1);
	StatusSwitcher->SetActiveWidgetIndex(Index);
	EnsureActiveVariantInitialized();
}

void UPCHeroStatusBarWidget::CopyVariantBySourceStatusBar(const UPCHeroStatusBarWidget* SourceStatusBar) const
{
	if (!SourceStatusBar || !StatusSwitcher || !SourceStatusBar->StatusSwitcher)
		return;

	const int32 SrcIndex = SourceStatusBar->StatusSwitcher->GetActiveWidgetIndex();
	StatusSwitcher->SetActiveWidgetIndex(SrcIndex);
	
	if (UPCUnitStatusBarWidget* ActiveStatusBar = Cast<UPCUnitStatusBarWidget>(StatusSwitcher->GetActiveWidget()))
	{
		const UAbilitySystemComponent* SrcASC = SourceStatusBar->ASC.Get();

		const float HP = SrcASC->GetNumericAttribute(SourceStatusBar->HealthAttr);
		const float MaxHP = SrcASC->GetNumericAttribute(SourceStatusBar->MaxHealthAttr);
		const float Mana = SrcASC->GetNumericAttribute(SourceStatusBar->ManaAttr);
		const float MaxMana = SrcASC->GetNumericAttribute(SourceStatusBar->MaxManaAttr);

		ActiveStatusBar->SetInstant(HP, MaxHP, Mana, MaxMana);
	}
}

void UPCHeroStatusBarWidget::HandleVisibilityChanged(ESlateVisibility NewVis)
{
	if (NewVis == ESlateVisibility::Visible)
	{
		if (UPCUnitStatusBarWidget* ActiveStatusBar = Cast<UPCUnitStatusBarWidget>(StatusSwitcher->GetActiveWidget()))
		{
			ActiveStatusBar->UpdateUI();
		}
	}
}

void UPCHeroStatusBarWidget::EnsureActiveVariantInitialized()
{
	if (!StatusSwitcher)
		return;

	if (UWidget* Active = StatusSwitcher->GetActiveWidget())
	{
		if (UPCUnitStatusBarWidget* Child = Cast<UPCUnitStatusBarWidget>(Active))
		{
			if (!InitializedStatusBarSet.Contains(Child))
			{
				Child->InitWithASC(ASC.Get(),
					HealthAttr, MaxHealthAttr,
					ManaAttr, MaxManaAttr);

				InitializedStatusBarSet.Add(Child);
			}
		}
	}
}
