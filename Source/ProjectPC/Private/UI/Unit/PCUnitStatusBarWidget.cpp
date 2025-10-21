// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Unit/PCUnitStatusBarWidget.h"

#include "AbilitySystemComponent.h"
#include "Character/Unit/PCBaseUnitCharacter.h"
#include "Component/PCUnitEquipmentComponent.h"
#include "Components/HorizontalBox.h"
#include "Components/Image.h"
#include "GameFramework/WorldSubsystem/PCItemManagerSubsystem.h"
#include "UI/Unit/PCUnitHealthProgressBar.h"


void UPCUnitStatusBarWidget::InitWithASC(APCBaseUnitCharacter* InUnit, UAbilitySystemComponent* InASC, FGameplayAttribute InHealthAttr,
                                         FGameplayAttribute InMaxHealthAttr, FGameplayAttribute InManaAttr, FGameplayAttribute InMaxManaAttr)
{
	Unit = InUnit;
	ASC = InASC;
	HealthAttr = InHealthAttr;
	MaxHealthAttr = InMaxHealthAttr;
	ManaAttr = InManaAttr;
	MaxManaAttr = InMaxManaAttr;

	bHasMana = InManaAttr.IsValid() && InMaxManaAttr.IsValid();

	if (!Unit.IsValid() || !ASC.IsValid())
	{
		ApplyToUI();
		return;
	}

	CachedHP = ASC->GetNumericAttribute(HealthAttr);
	CachedMaxHP = ASC->GetNumericAttribute(MaxHealthAttr);
	
	if (bHasMana)
	{
		CachedMP = ASC->GetNumericAttribute(ManaAttr);
		CachedMaxMP = ASC->GetNumericAttribute(MaxManaAttr);
	}
	ApplyToUI();

	// 델리게이트 연결
	HealthHandle = ASC->GetGameplayAttributeValueChangeDelegate(HealthAttr)
		.AddUObject(this, &ThisClass::OnHealthChanged);
	MaxHealthHandle = ASC->GetGameplayAttributeValueChangeDelegate(MaxHealthAttr)
		.AddUObject(this, &ThisClass::OnMaxHealthChanged);

	if (bHasMana)
	{
		ManaHandle = ASC->GetGameplayAttributeValueChangeDelegate(ManaAttr)
			.AddUObject(this, &ThisClass::OnManaChanged);
		MaxManaHandle = ASC->GetGameplayAttributeValueChangeDelegate(MaxManaAttr)
			.AddUObject(this, &ThisClass::OnMaxManaChanged);
	}

	if (Unit.IsValid())
	{
		if (UPCUnitEquipmentComponent* EquipmentComponent = Unit->GetEquipmentComponent())
		{
			EquipItemChangedHandle = EquipmentComponent->OnEquipItemChanged
				.AddUObject(this, &UPCUnitStatusBarWidget::OnEquipItemChanged);
		}
	}
}

void UPCUnitStatusBarWidget::SetInstant(APCBaseUnitCharacter* InUnit, float CurrentHP, float MaxHP, float CurrentMP, float MaxMP)
{
	Unit = InUnit;
	
	CachedHP = CurrentHP;
	CachedMaxHP = MaxHP;

	bHasMana = MaxMP > 0.f;

	if (bHasMana)
	{
		CachedMP = CurrentMP;
		CachedMaxMP = MaxMP;
	}

	ApplyToUI();
}

void UPCUnitStatusBarWidget::UpdateUI() const
{
	ApplyToUI();
}

void UPCUnitStatusBarWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	
	if (ItemImage_0 && ItemImage_1 && ItemImage_2)
	{
		EquipItemImages.Add(ItemImage_0);
		EquipItemImages.Add(ItemImage_1);
		EquipItemImages.Add(ItemImage_2);
	}
}

void UPCUnitStatusBarWidget::NativeDestruct()
{
	ClearDelegate();
	
	Super::NativeDestruct();
}

void UPCUnitStatusBarWidget::OnHealthChanged(const FOnAttributeChangeData& Data)
{
	// 값은 델리게이트 인자 대신, 항상 ASC에서 '지금 시점'의 값을 재조회
	const float NowValue = ASC.IsValid()
		? ASC->GetNumericAttribute(HealthAttr)
		: Data.NewValue;
	
	CachedHP = NowValue;
	UpdateHealthBar();
}

void UPCUnitStatusBarWidget::OnMaxHealthChanged(const FOnAttributeChangeData& Data)
{	
	// 값은 델리게이트 인자 대신, 항상 ASC에서 '지금 시점'의 값을 재조회
	const float NowValue = ASC.IsValid()
		? ASC->GetNumericAttribute(MaxHealthAttr)
		: Data.NewValue;
	
	CachedMaxHP = FMath::Max(1.f, NowValue);
	UpdateHealthBar();
} 

void UPCUnitStatusBarWidget::OnManaChanged(const FOnAttributeChangeData& Data)
{
	// 값은 델리게이트 인자 대신, 항상 ASC에서 '지금 시점'의 값을 재조회
	const float NowValue = ASC.IsValid()
		? ASC->GetNumericAttribute(ManaAttr)
		: Data.NewValue;
	
	CachedMP = NowValue;
	UpdateManaBar();
}

void UPCUnitStatusBarWidget::OnMaxManaChanged(const FOnAttributeChangeData& Data)
{
	// 값은 델리게이트 인자 대신, 항상 ASC에서 '지금 시점'의 값을 재조회
	const float NowValue = ASC.IsValid()
		? ASC->GetNumericAttribute(ManaAttr)
		: Data.NewValue;
	
	CachedMaxMP = FMath::Max(1.f, NowValue);
	UpdateManaBar();
}

void UPCUnitStatusBarWidget::OnEquipItemChanged() const
{
	UpdateEquipItemImages();
}

void UPCUnitStatusBarWidget::ApplyToUI() const
{
	UpdateHealthBar();
	UpdateManaBar();
	UpdateEquipItemImages();
}

void UPCUnitStatusBarWidget::UpdateHealthBar() const
{
	// CachedMaxHP, MP는 항상 0보다 크다는 것이 보장돼있기 때문에 나누기 연산에서 안전함 (따로 체크 안해도됨)
	if (HealthBar)
		HealthBar->SetValues(CachedHP, CachedMaxHP);
}

void UPCUnitStatusBarWidget::UpdateManaBar() const
{
	if (ManaBar && bHasMana)
		ManaBar->SetPercent(CachedMP / CachedMaxMP);
	else
	{
		ManaBar->SetPercent(0.f);
	}
}

void UPCUnitStatusBarWidget::UpdateEquipItemImages() const
{
	if (!Unit.IsValid())
		return;
	
	TArray<FGameplayTag> ItemTags = Unit->GetEquipItemTags();
	UPCItemManagerSubsystem* ItemManagerSubsystem = GetWorld() ? GetWorld()->GetSubsystem<UPCItemManagerSubsystem>() : nullptr;
	if (!ItemManagerSubsystem)
		return;
	
	for (int32 i=0; i<EquipItemImages.Num(); ++i)
	{
		if (ItemTags.IsValidIndex(i))
		{
			const FGameplayTag& ItemTag = ItemTags[i];
			if (ItemTag.IsValid())
			{
				if (UTexture2D* ItemTexture = ItemManagerSubsystem->GetItemTexture(ItemTag))
				{
					EquipItemImages[i]->SetBrushFromTexture(ItemTexture, true);
					EquipItemImages[i]->SetVisibility(ESlateVisibility::Visible);
					continue;
				}
			}
		}

		EquipItemImages[i]->SetBrushFromTexture(nullptr);
		EquipItemImages[i]->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UPCUnitStatusBarWidget::ClearDelegate()
{
	if (ASC.IsValid())
	{
		if (HealthHandle.IsValid())
		{
			ASC->GetGameplayAttributeValueChangeDelegate(HealthAttr).Remove(HealthHandle);
			HealthHandle.Reset();
		}
		if (MaxHealthHandle.IsValid())
		{
			ASC->GetGameplayAttributeValueChangeDelegate(MaxHealthAttr).Remove(MaxHealthHandle);
			MaxHealthHandle.Reset();
		}
		if (ManaHandle.IsValid())
		{
			ASC->GetGameplayAttributeValueChangeDelegate(ManaAttr).Remove(ManaHandle);
			ManaHandle.Reset();
		}
		if (MaxManaHandle.IsValid())
		{
			ASC->GetGameplayAttributeValueChangeDelegate(MaxManaAttr).Remove(MaxManaHandle);
			MaxManaHandle.Reset();
		}
	}

	if (Unit.IsValid() && EquipItemChangedHandle.IsValid())
	{
		if (UPCUnitEquipmentComponent* EquipmentComponent = Unit->GetEquipmentComponent())
		{
			EquipmentComponent->OnEquipItemChanged.Remove(EquipItemChangedHandle);
			EquipItemChangedHandle.Reset();
		}
	}
}

