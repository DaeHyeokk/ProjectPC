// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseGameplayTags.h"

namespace UnitGameplayTags
{
	// ==== Unit Types ====
	UE_DEFINE_GAMEPLAY_TAG(Unit_Type_Hero, "Unit.Type.Hero")
	UE_DEFINE_GAMEPLAY_TAG(Unit_Type_Creep, "Unit.Type.Creep")

	// ==== Unit Attack Types
	UE_DEFINE_GAMEPLAY_TAG(Unit_AttackType_Physical, "Unit.AttackType.Physical")
	UE_DEFINE_GAMEPLAY_TAG(Unit_AttackType_Magic, "Unit.AttackType.Magic")

	// ==== Unit Stat Tags ====
	UE_DEFINE_GAMEPLAY_TAG(Unit_Stat_Shared_MaxHealth, "Unit.Stat.Shared.MaxHealth")
	UE_DEFINE_GAMEPLAY_TAG(Unit_Stat_Shared_CurrentHealth, "Unit.Stat.Shared.CurrentHealth")
	UE_DEFINE_GAMEPLAY_TAG(Unit_Stat_Shared_BaseDamage, "Unit.Stat.Shared.BaseDamage")
	UE_DEFINE_GAMEPLAY_TAG(Unit_Stat_Shared_AttackRange, "Unit.Stat.Shared.AttackRange")
	UE_DEFINE_GAMEPLAY_TAG(Unit_Stat_Shared_AttackSpeed, "Unit.Stat.Shared.AttackSpeed")
	UE_DEFINE_GAMEPLAY_TAG(Unit_Stat_Shared_PhysicalDefense, "Unit.Stat.Shared.PhysicalDefense")
	UE_DEFINE_GAMEPLAY_TAG(Unit_Stat_Shared_MagicDefense, "Unit.Stat.Shared.MagicDefense")
	
	UE_DEFINE_GAMEPLAY_TAG(Unit_Stat_Hero_MaxMana, "Unit.Stat.Hero.MaxMana")
	UE_DEFINE_GAMEPLAY_TAG(Unit_Stat_Hero_CurrentMana, "Unit.Stat.Hero.CurrentMana")
	UE_DEFINE_GAMEPLAY_TAG(Unit_Stat_Hero_PhysicalDamageMultiplier, "Unit.Stat.Hero.PhysicalDamageMultiplier")
	UE_DEFINE_GAMEPLAY_TAG(Unit_Stat_Hero_MagicDamageMultiplier, "Unit.Stat.Hero.MagicDamageMultiplier")
	UE_DEFINE_GAMEPLAY_TAG(Unit_Stat_Hero_CritChance, "Unit.Stat.Hero.CritChance")
	UE_DEFINE_GAMEPLAY_TAG(Unit_Stat_Hero_LifeSteal, "Unit.Stat.Hero.LifeSteal")
	UE_DEFINE_GAMEPLAY_TAG(Unit_Stat_Hero_SpellVamp, "Unit.Stat.Hero.SpellVamp")
	
	// ==== Unit Action Ability Tags
	UE_DEFINE_GAMEPLAY_TAG(Unit_Action_Attack, "Unit.Action.Attack")
	UE_DEFINE_GAMEPLAY_TAG(Unit_Action_Attack_Ultimate, "Unit.Action.Attack.Ultimate")
}

namespace GameplayEffectTags
{
	UE_DEFINE_GAMEPLAY_TAG(GE_Class_InitDefaultStat, "GE.Class.InitDefaultStat")
	UE_DEFINE_GAMEPLAY_TAG(GE_Class_HealthChange, "GE.Class.HealthChange")
	UE_DEFINE_GAMEPLAY_TAG(GE_Class_ManaChange, "GE.Class.ManaChange")

	UE_DEFINE_GAMEPLAY_TAG(GE_Caller_CurrentHealthChange, "GE.Caller.CurrentHealthChange")
	UE_DEFINE_GAMEPLAY_TAG(GE_Caller_MaxHealthChange, "GE.Caller.MaxHealthChange")
	UE_DEFINE_GAMEPLAY_TAG(GE_Caller_ManaChange, "GE.Caller.ManaChange")
	UE_DEFINE_GAMEPLAY_TAG(GE_Caller_BaseDamageChange, "GE.Caller.BaseDamageChange")
	UE_DEFINE_GAMEPLAY_TAG(GE_Caller_PhysicalDamageMultiplierChange, "GE.Caller.PhysicalDamageMultiplierChange")
	UE_DEFINE_GAMEPLAY_TAG(GE_Caller_MagicDamageMultiplierChange, "GE.Caller.MagicDamageMultiplierChange")
	UE_DEFINE_GAMEPLAY_TAG(GE_Caller_AttackSpeedChange, "GE.Caller.AttackSpeedChange")
	UE_DEFINE_GAMEPLAY_TAG(GE_Caller_PhysicalDefenseChange, "GE.Caller.PhysicalDefenseChange")
	UE_DEFINE_GAMEPLAY_TAG(GE_Caller_MagicDefenseChange, "GE.Caller.MagicDefenseChange")
	UE_DEFINE_GAMEPLAY_TAG(GE_Caller_CritChanceChange, "GE.Caller.CritChangeChange")
	UE_DEFINE_GAMEPLAY_TAG(GE_Caller_LifeStealChange, "GE.Caller.LifeStealChange")
	UE_DEFINE_GAMEPLAY_TAG(GE_Caller_SpellVampChange, "GE.Caller.SpellVampChange")
}

