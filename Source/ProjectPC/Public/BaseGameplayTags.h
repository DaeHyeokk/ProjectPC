// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "NativeGameplayTags.h"

namespace GameStateTags
{
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Game_State)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Game_State_NonCombat)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Game_State_Combat)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Game_State_Combat_Preparation)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Game_State_Combat_Active)	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Game_State_Combat_End)
}

namespace UnitGameplayTags
{
	// ==== Unit Types ====
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Unit_Type_Hero)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Unit_Type_Hero_Sparrow)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Unit_Type_Hero_Drongo)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Unit_Type_Creep)

	// ==== Unit Attack Types ====
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Unit_DamageType_Physical)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Unit_DamageType_Magic)

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
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Unit_Action_Attack_Basic)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Unit_Action_Attack_Ultimate)

	// ==== Unit State Tags ====
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Unit_State_Combat_Alive)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Unit_State_Combat_Dead)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Unit_State_Combat_Attacking)

	// ==== Unit Cooldown Tags ====
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Unit_Cooldown_BasicAttack)
	
	// ==== Unit Anim Montage Tags ====
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Unit_Montage_LevelStart)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Unit_Montage_Attack_Basic)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Unit_Montage_Attack_Ultimate)

	// ==== Unit GameplayEvent Tags ====
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Unit_Event_AttackCommit)
}

namespace SynergyGameplayTags
{
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Synergy_Species_Human)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Synergy_Species_Demon)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Synergy_Species_Imperial)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Synergy_Species_Robot)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Synergy_Species_Cyborg)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Synergy_Species_Beast)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Synergy_Species_Undead)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Synergy_Species_Darkness)

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Synergy_Job_Marksman)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Synergy_Job_Mechanic)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Synergy_Job_BladeMaster)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Synergy_Job_Knight)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Synergy_Job_Mage)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Synergy_Job_Guardian)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Synergy_Job_Assassin)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Synergy_Job_Bruiser)
}

namespace GameplayEffectTags
{
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GE_Class_Health_Heal_Instant)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GE_Class_Health_Damage_Instant)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GE_Class_Mana_Gain_Instant)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GE_Class_Mana_Spend_Instant)
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GE_Class_Cooldown_BasicAttack)

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
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GE_Caller_Cooldown_BasicAttack)
}

namespace PlayerGameplayTags
{
	// ==== Player Stat Tags ====
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Player_Stat_PlayerLevel)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Player_Stat_PlayerXP)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Player_Stat_PlayerGold)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Player_Stat_PlayerHP)

	// ==== Player State Tags ====
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Player_State_Normal)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Player_State_Carousel)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Player_State_Dead)

	// ==== Player Stat Change GE Tags ====
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Player_GE_LevelChange)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Player_GE_XPChange)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Player_GE_GoldChange)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Player_GE_HPChange)
	
	// ==== Player Shop Ability Tags ====
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Player_GA_Shop_BuyXP)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Player_GA_Shop_BuyUnit)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Player_GA_Shop_SellUnit)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Player_GA_Shop_ShopRefresh)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Player_GA_Shop_ShopLock)
}