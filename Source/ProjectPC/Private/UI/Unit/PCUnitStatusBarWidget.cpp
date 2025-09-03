// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Unit/PCUnitStatusBarWidget.h"

#include "AbilitySystemComponent.h"
#include "Components/TextBlock.h"
#include "UI/Unit/PCUnitHealthProgressBar.h"


void UPCUnitStatusBarWidget::InitWithASC(UAbilitySystemComponent* InASC, FGameplayAttribute InHealthAttr,
                                         FGameplayAttribute InMaxHealthAttr, FGameplayAttribute InManaAttr, FGameplayAttribute InMaxManaAttr)
{
	if (UAbilitySystemComponent* OldASC = ASC.Get())
	{
		if (HealthHandle.IsValid())
			OldASC->GetGameplayAttributeValueChangeDelegate(HealthAttr).Remove(HealthHandle);
		if (MaxHealthHandle.IsValid())
			OldASC->GetGameplayAttributeValueChangeDelegate(MaxHealthAttr).Remove(MaxHealthHandle);
		if (ManaHandle.IsValid())
			OldASC->GetGameplayAttributeValueChangeDelegate(ManaAttr).Remove(ManaHandle);
		if (MaxManaHandle.IsValid())
			OldASC->GetGameplayAttributeValueChangeDelegate(MaxManaAttr).Remove(MaxManaHandle);
	}
	
	ASC = InASC;
	HealthAttr = InHealthAttr;
	MaxHealthAttr = InMaxHealthAttr;
	ManaAttr = InManaAttr;
	MaxManaAttr = InMaxManaAttr;

	bHasMana = InManaAttr.IsValid() && InMaxManaAttr.IsValid();

	if (!ASC.IsValid())
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
}

void UPCUnitStatusBarWidget::SetInstant(float CurrentHP, float MaxHP, float CurrentMP, float MaxMP)
{
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

void UPCUnitStatusBarWidget::NativeDestruct()
{
	if (ASC.IsValid())
	{
		if (HealthHandle.IsValid())
			ASC->GetGameplayAttributeValueChangeDelegate(HealthAttr).Remove(HealthHandle);
		
		if (MaxHealthHandle.IsValid())
			ASC->GetGameplayAttributeValueChangeDelegate(MaxHealthAttr).Remove(MaxHealthHandle);
		
		if (ManaHandle.IsValid())
			ASC->GetGameplayAttributeValueChangeDelegate(ManaAttr).Remove(ManaHandle);
		
		if (MaxManaHandle.IsValid())
			ASC->GetGameplayAttributeValueChangeDelegate(MaxManaAttr).Remove(MaxManaHandle);
	}
	
	Super::NativeDestruct();
}

void UPCUnitStatusBarWidget::OnHealthChanged(const FOnAttributeChangeData& Data)
{
	UE_LOG(LogTemp, Warning, TEXT("UI OnCurrentHealthChanged: New=%.1f"), Data.NewValue);
	
	// 값은 델리게이트 인자 대신, 항상 ASC에서 '지금 시점'의 값을 재조회
	const float NowValue = ASC.IsValid()
		? ASC->GetNumericAttribute(HealthAttr)
		: Data.NewValue;
	
	CachedHP = NowValue;
	ApplyToUI();
}

void UPCUnitStatusBarWidget::OnMaxHealthChanged(const FOnAttributeChangeData& Data)
{
	UE_LOG(LogTemp, Warning, TEXT("UI OnMaxHealthChanged: New=%.1f"), Data.NewValue);
	
	// 값은 델리게이트 인자 대신, 항상 ASC에서 '지금 시점'의 값을 재조회
	const float NowValue = ASC.IsValid()
		? ASC->GetNumericAttribute(MaxHealthAttr)
		: Data.NewValue;
	
	CachedMaxHP = FMath::Max(1.f, NowValue);
	ApplyToUI();
} 

void UPCUnitStatusBarWidget::OnManaChanged(const FOnAttributeChangeData& Data)
{
	// 값은 델리게이트 인자 대신, 항상 ASC에서 '지금 시점'의 값을 재조회
	const float NowValue = ASC.IsValid()
		? ASC->GetNumericAttribute(ManaAttr)
		: Data.NewValue;
	
	CachedMP = NowValue;
	ApplyToUI();
}

void UPCUnitStatusBarWidget::OnMaxManaChanged(const FOnAttributeChangeData& Data)
{
	// 값은 델리게이트 인자 대신, 항상 ASC에서 '지금 시점'의 값을 재조회
	const float NowValue = ASC.IsValid()
		? ASC->GetNumericAttribute(ManaAttr)
		: Data.NewValue;
	
	CachedMaxMP = FMath::Max(1.f, NowValue);
	ApplyToUI();
}

void UPCUnitStatusBarWidget::ApplyToUI() const
{
	// CachedMaxHP, MP는 항상 0보다 크다는 것이 보장돼있기 때문에 나누기 연산에서 안전함 (따로 체크 안해도됨)
	if (HealthBar)
		HealthBar->SetValues(CachedHP, CachedMaxHP);
	
	if (ManaBar && bHasMana)
		ManaBar->SetPercent(CachedMP / CachedMaxMP);

	HealthText->SetText(FText::FromString(
		FString::Printf(TEXT("%.0f / %.0f"), CachedHP, CachedMaxHP)));
}
