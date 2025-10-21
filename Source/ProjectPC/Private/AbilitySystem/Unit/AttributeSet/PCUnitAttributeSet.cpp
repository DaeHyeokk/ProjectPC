// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Unit/AttributeSet/PCUnitAttributeSet.h"
#include "GameplayEffectExtension.h"
#include "Character/Unit/PCBaseUnitCharacter.h"
#include "Net/UnrealNetwork.h"

UPCUnitAttributeSet::UPCUnitAttributeSet()
{
	InitAttackSpeedIncreaseMultiplier(0.f);
	InitAttackSpeedDecreaseMultiplier(0.f);
}

float UPCUnitAttributeSet::GetEffectiveAttackSpeed() const
{
	const float Base = GetAttackSpeed();
	const float IncPercent = FMath::Max(0.f, GetAttackSpeedIncreaseMultiplier());
	const float DecPercent = FMath::Clamp(GetAttackSpeedDecreaseMultiplier(), 0.f, 100.f);

	const float IncFactor = 1.f + (IncPercent * 0.01f);
	const float DecFactor = 1.f - (DecPercent * 0.01f);

	const float Effective = Base * IncFactor * DecFactor;
	return FMath::Max(0.f, Effective);
}

void UPCUnitAttributeSet::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UPCUnitAttributeSet,MaxHealth, COND_None, REPNOTIFY_OnChanged);
	DOREPLIFETIME_CONDITION_NOTIFY(UPCUnitAttributeSet,CurrentHealth, COND_None, REPNOTIFY_OnChanged);
	DOREPLIFETIME_CONDITION_NOTIFY(UPCUnitAttributeSet,BaseDamage, COND_None, REPNOTIFY_OnChanged);
	DOREPLIFETIME_CONDITION_NOTIFY(UPCUnitAttributeSet,AttackRange, COND_None, REPNOTIFY_OnChanged);
	DOREPLIFETIME_CONDITION_NOTIFY(UPCUnitAttributeSet,AttackSpeed, COND_None, REPNOTIFY_OnChanged);
	DOREPLIFETIME_CONDITION_NOTIFY(UPCUnitAttributeSet,AttackSpeedIncreaseMultiplier, COND_None, REPNOTIFY_OnChanged);
	DOREPLIFETIME_CONDITION_NOTIFY(UPCUnitAttributeSet,AttackSpeedDecreaseMultiplier, COND_None, REPNOTIFY_OnChanged);
	DOREPLIFETIME_CONDITION_NOTIFY(UPCUnitAttributeSet,PhysicalDefense, COND_None, REPNOTIFY_OnChanged);
	DOREPLIFETIME_CONDITION_NOTIFY(UPCUnitAttributeSet,MagicDefense, COND_None, REPNOTIFY_OnChanged);
}

void UPCUnitAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetMaxHealthAttribute())
	{
		AdjustAttributeForMaxChange(CurrentHealth, MaxHealth, NewValue, GetCurrentHealthAttribute());
	}
	else if (Attribute == GetEvasionChanceAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, 100.f);
	}
	else if (Attribute == GetFlatDamageBlockAttribute()) // || Attribute == GetShieldAttribute()
	{
		NewValue = FMath::Max(0.f, NewValue);
	}
}

void UPCUnitAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetCurrentHealthAttribute())
	{
		const float ClampedHealth = FMath::Clamp(GetCurrentHealth(), 0.0f, GetMaxHealth());
		SetCurrentHealth(ClampedHealth);

		if (ClampedHealth <= 0.f)
		{
			APCBaseUnitCharacter* OwnerUnit = Cast<APCBaseUnitCharacter>(GetOwningActor());
			OwnerUnit->Die();
		}
	}
	// else if (Data.EvaluatedData.Attribute == GetShieldAttribute())
	// {
	// 	// 음수 방지
	// 	SetShield(FMath::Max(0.f, GetShield()));
	// }
}

void UPCUnitAttributeSet::AdjustAttributeForMaxChange(const FGameplayAttributeData& AffectedAttribute,
                                                      const FGameplayAttributeData& MaxAttribute, float NewMaxValue,
                                                      const FGameplayAttribute& AffectedAttributeProperty) const
 {
	UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();
	const float CurrentMaxValue = MaxAttribute.GetCurrentValue();
	if (!FMath::IsNearlyEqual(CurrentMaxValue, NewMaxValue) && ASC)
	{
		const float CurrentValue = AffectedAttribute.GetCurrentValue();
		float NewDelta = CurrentMaxValue > 0.f ? (CurrentValue * NewMaxValue / CurrentMaxValue) - CurrentValue : NewMaxValue;
		ASC->ApplyModToAttribute(AffectedAttributeProperty, EGameplayModOp::Additive, NewDelta);
	}
 }

void UPCUnitAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPCUnitAttributeSet, MaxHealth, OldMaxHealth);
}

void UPCUnitAttributeSet::OnRep_CurrentHealth(const FGameplayAttributeData& OldCurrentHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPCUnitAttributeSet, CurrentHealth, OldCurrentHealth);
}

void UPCUnitAttributeSet::OnRep_BaseDamage(const FGameplayAttributeData& OldBaseDamage)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPCUnitAttributeSet, BaseDamage, OldBaseDamage);
}

void UPCUnitAttributeSet::OnRep_AttackRange(const FGameplayAttributeData& OldAttackRange)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPCUnitAttributeSet, AttackRange, OldAttackRange);
}

void UPCUnitAttributeSet::OnRep_AttackSpeed(const FGameplayAttributeData& OldAttackSpeed)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPCUnitAttributeSet, AttackSpeed, OldAttackSpeed);
}

void UPCUnitAttributeSet::OnRep_AttackSpeedIncreaseMultiplier(
	const FGameplayAttributeData& OldAttackSpeedIncreaseMultiplier)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPCUnitAttributeSet, AttackSpeedIncreaseMultiplier, OldAttackSpeedIncreaseMultiplier);
}

void UPCUnitAttributeSet::OnRep_AttackSpeedDecreaseMultiplier(
	const FGameplayAttributeData& OldAttackSpeedDecreaseMultiplier)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPCUnitAttributeSet, AttackSpeedDecreaseMultiplier, OldAttackSpeedDecreaseMultiplier);
}

void UPCUnitAttributeSet::OnRep_PhysicalDefense(const FGameplayAttributeData& OldPhysicalDefense)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPCUnitAttributeSet, PhysicalDefense, OldPhysicalDefense);
}

void UPCUnitAttributeSet::OnRep_MagicDefense(const FGameplayAttributeData& OldMagicDefense)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPCUnitAttributeSet, MagicDefense, OldMagicDefense);
}