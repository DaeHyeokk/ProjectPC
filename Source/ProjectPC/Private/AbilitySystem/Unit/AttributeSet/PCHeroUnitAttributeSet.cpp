// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Unit/AttributeSet/PCHeroUnitAttributeSet.h"

#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"


UPCHeroUnitAttributeSet::UPCHeroUnitAttributeSet()
{
	//InitCritChance(25.f);
	InitCritChance(100.f);
	InitCritMultiplier(30.f);
}

void UPCHeroUnitAttributeSet::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UPCHeroUnitAttributeSet,MaxMana, COND_None, REPNOTIFY_OnChanged);
	DOREPLIFETIME_CONDITION_NOTIFY(UPCHeroUnitAttributeSet,CurrentMana, COND_None, REPNOTIFY_OnChanged);
	DOREPLIFETIME_CONDITION_NOTIFY(UPCHeroUnitAttributeSet,CombatStartMana, COND_None, REPNOTIFY_OnChanged);
	DOREPLIFETIME_CONDITION_NOTIFY(UPCHeroUnitAttributeSet,ManaRegen, COND_None, REPNOTIFY_OnChanged);
	DOREPLIFETIME_CONDITION_NOTIFY(UPCHeroUnitAttributeSet,UltimateDamage, COND_None, REPNOTIFY_OnChanged);
	DOREPLIFETIME_CONDITION_NOTIFY(UPCHeroUnitAttributeSet,UltimateCost, COND_None, REPNOTIFY_OnChanged);
	DOREPLIFETIME_CONDITION_NOTIFY(UPCHeroUnitAttributeSet,PhysicalDamageMultiplier, COND_None, REPNOTIFY_OnChanged);
	DOREPLIFETIME_CONDITION_NOTIFY(UPCHeroUnitAttributeSet,MagicDamageMultiplier, COND_None, REPNOTIFY_OnChanged);
	DOREPLIFETIME_CONDITION_NOTIFY(UPCHeroUnitAttributeSet,DamageMultiplier, COND_None, REPNOTIFY_OnChanged);
	DOREPLIFETIME_CONDITION_NOTIFY(UPCHeroUnitAttributeSet,CritChance, COND_None, REPNOTIFY_OnChanged);
	DOREPLIFETIME_CONDITION_NOTIFY(UPCHeroUnitAttributeSet,CritMultiplier, COND_None, REPNOTIFY_OnChanged);
	DOREPLIFETIME_CONDITION_NOTIFY(UPCHeroUnitAttributeSet,LifeSteal, COND_None, REPNOTIFY_OnChanged);
}

void UPCHeroUnitAttributeSet::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetCurrentManaAttribute())
	{
		SetCurrentMana(FMath::Clamp(GetCurrentMana(), 0.0f, GetMaxMana()));		
	}
}

void UPCHeroUnitAttributeSet::OnRep_MaxMana(const FGameplayAttributeData& OldMaxMana)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPCHeroUnitAttributeSet, MaxMana, OldMaxMana);
}

void UPCHeroUnitAttributeSet::OnRep_CurrentMana(const FGameplayAttributeData& OldCurrentMana)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPCHeroUnitAttributeSet, CurrentMana, OldCurrentMana);
}

void UPCHeroUnitAttributeSet::OnRep_CombatStartMana(const FGameplayAttributeData& OldCombatStartMana)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPCHeroUnitAttributeSet, CombatStartMana, OldCombatStartMana);
}

void UPCHeroUnitAttributeSet::OnRep_ManaRegen(const FGameplayAttributeData& OldManaRegen)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPCHeroUnitAttributeSet, ManaRegen, OldManaRegen);
}

void UPCHeroUnitAttributeSet::OnRep_UltimateDamage(const FGameplayAttributeData& OldUltimateDamage)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPCHeroUnitAttributeSet, UltimateDamage, OldUltimateDamage);
}

void UPCHeroUnitAttributeSet::OnRep_UltimateCost(const FGameplayAttributeData& OldUltimateCost)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPCHeroUnitAttributeSet, UltimateCost, OldUltimateCost);
}

void UPCHeroUnitAttributeSet::OnRep_PhysicalDamageMultiplier(const FGameplayAttributeData& OldPhysicalDamageMultiplier)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPCHeroUnitAttributeSet, PhysicalDamageMultiplier, OldPhysicalDamageMultiplier);
}

void UPCHeroUnitAttributeSet::OnRep_MagicDamageMultiplier(const FGameplayAttributeData& OldMagicDamageMultiplier)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPCHeroUnitAttributeSet, MagicDamageMultiplier, OldMagicDamageMultiplier);
}

void UPCHeroUnitAttributeSet::OnRep_DamageMultiplier(const FGameplayAttributeData& OldDamageMultiplier)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPCHeroUnitAttributeSet, DamageMultiplier, OldDamageMultiplier);
}

void UPCHeroUnitAttributeSet::OnRep_CritChance(const FGameplayAttributeData& OldCritChance)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPCHeroUnitAttributeSet, CritChance, OldCritChance);
}

void UPCHeroUnitAttributeSet::OnRep_CritMultiplier(const FGameplayAttributeData& OldCritMultiplier)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPCHeroUnitAttributeSet, CritMultiplier, OldCritMultiplier);
}

void UPCHeroUnitAttributeSet::OnRep_LifeSteal(const FGameplayAttributeData& OldLifeSteal)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPCHeroUnitAttributeSet, LifeSteal, OldLifeSteal);
}