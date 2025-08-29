// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Unit/AttributeSet/PCUnitAttributeSet.h"
#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"

void UPCUnitAttributeSet::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UPCUnitAttributeSet,MaxHealth);
	DOREPLIFETIME(UPCUnitAttributeSet,CurrentHealth);
	DOREPLIFETIME(UPCUnitAttributeSet,BaseDamage);
	DOREPLIFETIME(UPCUnitAttributeSet,AttackRange);
	DOREPLIFETIME(UPCUnitAttributeSet,AttackSpeed);
	DOREPLIFETIME(UPCUnitAttributeSet,PhysicalDefense);
	DOREPLIFETIME(UPCUnitAttributeSet,MagicDefense);
}

void UPCUnitAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetMaxHealthAttribute())
	{
		AdjustAttributeForMaxChange(CurrentHealth, MaxHealth, NewValue, GetCurrentHealthAttribute());
	}
}

void UPCUnitAttributeSet::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetCurrentHealthAttribute())
	{
		SetCurrentHealth(FMath::Clamp(GetCurrentHealth(), 0.0f, GetMaxHealth()));
	}
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
		ASC->ApplyModToAttributeUnsafe(AffectedAttributeProperty, EGameplayModOp::Additive, NewDelta);
	}
}

void UPCUnitAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth)
{
	UE_LOG(LogTemp, Warning, TEXT("Client OnRep_MaxHealth: Old=%.1f New=%.1f"),
	OldMaxHealth.GetCurrentValue(), MaxHealth.GetCurrentValue());
	//GAMEPLAYATTRIBUTE_REPNOTIFY(UPCUnitAttributeSet, MaxHealth, OldMaxHealth);
}

void UPCUnitAttributeSet::OnRep_CurrentHealth(const FGameplayAttributeData& OldCurrentHealth)
{
	UE_LOG(LogTemp, Warning, TEXT("Client OnRep_CurrentHealth: Old=%.1f New=%.1f"),
OldCurrentHealth.GetCurrentValue(), CurrentHealth.GetCurrentValue());
	//GAMEPLAYATTRIBUTE_REPNOTIFY(UPCUnitAttributeSet, CurrentHealth, OldCurrentHealth);
}

void UPCUnitAttributeSet::OnRep_BaseDamage(const FGameplayAttributeData& OldBaseDamage)
{
	//GAMEPLAYATTRIBUTE_REPNOTIFY(UPCUnitAttributeSet, BaseDamage, OldBaseDamage);
}

void UPCUnitAttributeSet::OnRep_AttackRange(const FGameplayAttributeData& OldAttackRange)
{
	//GAMEPLAYATTRIBUTE_REPNOTIFY(UPCUnitAttributeSet, AttackRange, OldAttackRange);
}

void UPCUnitAttributeSet::OnRep_AttackSpeed(const FGameplayAttributeData& OldAttackSpeed)
{
	//GAMEPLAYATTRIBUTE_REPNOTIFY(UPCUnitAttributeSet, AttackSpeed, OldAttackSpeed);
}

void UPCUnitAttributeSet::OnRep_PhysicalDefense(const FGameplayAttributeData& OldPhysicalDefense)
{
	//GAMEPLAYATTRIBUTE_REPNOTIFY(UPCUnitAttributeSet, PhysicalDefense, OldPhysicalDefense);
}

void UPCUnitAttributeSet::OnRep_MagicDefense(const FGameplayAttributeData& OldMagicDefense)
{
	//GAMEPLAYATTRIBUTE_REPNOTIFY(UPCUnitAttributeSet, MagicDefense, OldMagicDefense);
}