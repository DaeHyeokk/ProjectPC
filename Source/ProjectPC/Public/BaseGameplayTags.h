// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "NativeGameplayTags.h"

namespace UnitGameplayTags
{
	// ==== Unit Types ====
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Unit_Type_Hero)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Unit_Type_Creep)

	// ==== Unit Attack Types ====
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Unit_AttackType_Physical)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Unit_AttackType_Magic)

	// ==== Unit Stat Tags ====
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Unit_Stat_Shared_MaxHealth)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Unit_Stat_Shared_CurrentHealth)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Unit_Stat_Shared_BaseDamage)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Unit_Stat_Shared_AttackRange)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Unit_Stat_Shared_AttackSpeed)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Unit_Stat_Shared_PhysicalDefense)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Unit_Stat_Shared_MagicDefense)
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Unit_Stat_Hero_MaxMana)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Unit_Stat_Hero_CurrentMana)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Unit_Stat_Hero_PhysicalDamageMultiplier)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Unit_Stat_Hero_MagicDamageMultiplier)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Unit_Stat_Hero_CritChance)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Unit_Stat_Hero_LifeSteal)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Unit_Stat_Hero_SpellVamp)

	// ==== Unit Action Ability Tags ====
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Unit_Action_Attack)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Unit_Action_Attack_Ultimate)
}

namespace GameplayEffectTags
{
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GE_Class_InitDefaultStat)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GE_Class_HealthChange)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GE_Class_ManaChange)

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GE_Caller_CurrentHealthChange)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GE_Caller_MaxHealthChange)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GE_Caller_ManaChange)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GE_Caller_BaseDamageChange)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GE_Caller_PhysicalDamageMultiplierChange)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GE_Caller_MagicDamageMultiplierChange)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GE_Caller_AttackSpeedChange)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GE_Caller_PhysicalDefenseChange)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GE_Caller_MagicDefenseChange)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GE_Caller_CritChanceChange)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GE_Caller_LifeStealChange)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GE_Caller_SpellVampChange)
}

