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
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Unit_Type_Hero_Raven)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Unit_Type_Hero_Drongo)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Unit_Type_Hero_Greystone)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Unit_Type_Hero_Yin)
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Unit_Type_Hero_Revenant)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Unit_Type_Hero_TwinBlast)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Unit_Type_Hero_Murdock)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Unit_Type_Hero_Zinx)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Unit_Type_Creep)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Unit_Type_Creep_MinionLv1)

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
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Unit_Action)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Unit_Action_Attack)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Unit_Action_Attack_Basic)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Unit_Action_Attack_Ultimate)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Unit_Action_Death)

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
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Unit_Montage_Death)

	// ==== Unit GameplayEvent Tags ====
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Unit_Event_SpawnProjectileSucceed)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Unit_Event_HitSucceed)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Unit_Event_OnHit)
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
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GE_Class_HealthChange_Instant)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GE_Class_ManaChange_Instant)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GE_Class_Unit_Damage_Instant)
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GE_Class_Cooldown_BasicAttack)

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GE_Caller_Damage)
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GE_Caller_Stat_CurrentHealth)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GE_Caller_Stat_MaxHealth)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GE_Caller_Stat_CurrentMana)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GE_Caller_Stat_MaxMana)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GE_Caller_Stat_BaseDamage)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GE_Caller_Stat_PhysicalDamageMultiplier)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GE_Caller_Stat_MagicDamageMultiplier)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GE_Caller_Stat_AttackSpeed)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GE_Caller_Stat_PhysicalDefense)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GE_Caller_Stat_MagicDefense)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GE_Caller_Stat_CritChance)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GE_Caller_Stat_LifeSteal)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GE_Caller_Stat_SpellVamp)
	
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
	
	// ==== Player Shop Ability Tags ====
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Player_GA_Shop_BuyXP)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Player_GA_Shop_BuyUnit)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Player_GA_Shop_SellUnit)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Player_GA_Shop_ShopRefresh)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Player_GA_Shop_ShopLock)

	// ==== Player Event Tags ====
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Player_Event_Damage)
}