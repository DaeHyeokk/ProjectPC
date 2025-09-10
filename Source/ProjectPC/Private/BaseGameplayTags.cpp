// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseGameplayTags.h"

namespace GameStateTags
{
	UE_DEFINE_GAMEPLAY_TAG(Game_State, "Game.State")	
	UE_DEFINE_GAMEPLAY_TAG(Game_State_NonCombat, "Game.State.NonCombat")	
	UE_DEFINE_GAMEPLAY_TAG(Game_State_Combat, "Game.State.Combat")	
	UE_DEFINE_GAMEPLAY_TAG(Game_State_Combat_Preparation, "Game.State.Combat.Preparation")	
	UE_DEFINE_GAMEPLAY_TAG(Game_State_Combat_Active, "Game.State.Combat.Active")	
	UE_DEFINE_GAMEPLAY_TAG(Game_State_Combat_End, "Game.State.Combat.End")	
}

namespace UnitGameplayTags
{
	// ==== Unit Types ====
	UE_DEFINE_GAMEPLAY_TAG(Unit_Type_Hero, "Unit.Type.Hero")
	UE_DEFINE_GAMEPLAY_TAG(Unit_Type_Hero_Sparrow, "Unit.Type.Hero.Sparrow")
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
	UE_DEFINE_GAMEPLAY_TAG(Unit_Action_Attack_Basic, "Unit.Action.Attack.Basic")
	UE_DEFINE_GAMEPLAY_TAG(Unit_Action_Attack_Ultimate, "Unit.Action.Attack.Ultimate")

	// ==== Unit State Tags ====
	UE_DEFINE_GAMEPLAY_TAG(Unit_State_Combat_Alive, "Unit.State.Combat.Alive")
	UE_DEFINE_GAMEPLAY_TAG(Unit_State_Combat_Death, "Unit.State.Combat.Death")
	
	// ==== Unit Anim Montage Tags ====
	UE_DEFINE_GAMEPLAY_TAG(Unit_Montage_LevelStart, "Unit.Montage.LevelStart")
}

namespace SynergyGameplayTags
{
	UE_DEFINE_GAMEPLAY_TAG(Synergy_Species_Human, "Synergy.Species.Human")
	UE_DEFINE_GAMEPLAY_TAG(Synergy_Species_Demon, "Synergy.Species.Demon")
	UE_DEFINE_GAMEPLAY_TAG(Synergy_Species_Imperial, "Synergy.Species.Imperial")
	UE_DEFINE_GAMEPLAY_TAG(Synergy_Species_Robot, "Synergy.Species.Robot")
	UE_DEFINE_GAMEPLAY_TAG(Synergy_Species_Cyborg, "Synergy.Species.Cyborg")
	UE_DEFINE_GAMEPLAY_TAG(Synergy_Species_Beast, "Synergy.Species.Beast")
	UE_DEFINE_GAMEPLAY_TAG(Synergy_Species_Undead, "Synergy.Species.Undead")
	UE_DEFINE_GAMEPLAY_TAG(Synergy_Species_Darkness, "Synergy.Species.Darkness")
	
	UE_DEFINE_GAMEPLAY_TAG(Synergy_Job_Marksman, "Synergy.Job.Marksman")
	UE_DEFINE_GAMEPLAY_TAG(Synergy_Job_Mechanic, "Synergy.Job.Mechanic")
	UE_DEFINE_GAMEPLAY_TAG(Synergy_Job_BladeMaster, "Synergy.Job.BladeMaster")
	UE_DEFINE_GAMEPLAY_TAG(Synergy_Job_Knight, "Synergy.Job.Knight")
	UE_DEFINE_GAMEPLAY_TAG(Synergy_Job_Mage, "Synergy.Job.Mage")
	UE_DEFINE_GAMEPLAY_TAG(Synergy_Job_Guardian, "Synergy.Job.Guardian")
	UE_DEFINE_GAMEPLAY_TAG(Synergy_Job_Assassin, "Synergy.Job.Assassin")
	UE_DEFINE_GAMEPLAY_TAG(Synergy_Job_Bruiser, "Synergy.Job.Bruiser")
}

namespace GameplayEffectTags
{
	UE_DEFINE_GAMEPLAY_TAG(GE_Class_HealthChange, "GE.Class.HealthChange")
	UE_DEFINE_GAMEPLAY_TAG(GE_Class_ManaChange, "GE.Class.ManaChange")

	UE_DEFINE_GAMEPLAY_TAG(GE_Class_Cooldown_BasicAttack, "GE.Class.Cooldown.BasicAttack")
	
	UE_DEFINE_GAMEPLAY_TAG(GE_Class_GrantUnitTypeTag_Hero, "GE.Class.GrantUnitTypeTag.Hero")
	UE_DEFINE_GAMEPLAY_TAG(GE_Class_GrantUnitTypeTag_Creep, "GE.Class.GrantUnitTypeTag.Creep")

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
	
	UE_DEFINE_GAMEPLAY_TAG(GE_Caller_Cooldown_BasicAttack, "GE.Caller.Cooldown.BasicAttack")
}

namespace PlayerGameplayTags
{
	// ==== Player Stat Tags ====
	UE_DEFINE_GAMEPLAY_TAG(Player_Stat_PlayerLevel, "Player.Stat.PlayerLevel")
	UE_DEFINE_GAMEPLAY_TAG(Player_Stat_PlayerXP, "Player.Stat.PlayerXP")
	UE_DEFINE_GAMEPLAY_TAG(Player_Stat_PlayerGold, "Player.Stat.PlayerGold")
	UE_DEFINE_GAMEPLAY_TAG(Player_Stat_PlayerHP, "Player.Stat.PlayerHP")

	// ==== Player State Tags ====
	UE_DEFINE_GAMEPLAY_TAG(Player_State_Normal, "Player.State.Normal")
	UE_DEFINE_GAMEPLAY_TAG(Player_State_Carousel, "Player.State.Carousel")
	UE_DEFINE_GAMEPLAY_TAG(Player_State_Dead, "Player.State.Dead")
	
	// ==== Player Stat Change GE Tags ====
	UE_DEFINE_GAMEPLAY_TAG(Player_GE_LevelChange, "Player.GE.LevelChange")
	UE_DEFINE_GAMEPLAY_TAG(Player_GE_XPChange, "Player.GE.XPChange")
	UE_DEFINE_GAMEPLAY_TAG(Player_GE_GoldChange, "Player.GE.GoldChange")
	UE_DEFINE_GAMEPLAY_TAG(Player_GE_HPChange, "Player.GE.HPChange")

	// ==== Player Shop Ability Tags ====
	UE_DEFINE_GAMEPLAY_TAG(Player_GA_Shop_BuyXP, "Player.GA.Shop.BuyXP")
	UE_DEFINE_GAMEPLAY_TAG(Player_GA_Shop_BuyUnit, "Player.GA.Shop.BuyUnit")
	UE_DEFINE_GAMEPLAY_TAG(Player_GA_Shop_SellUnit, "Player.GA.Shop.SellUnit")
	UE_DEFINE_GAMEPLAY_TAG(Player_GA_Shop_ShopRefresh, "Player.GA.Shop.ShopRefresh")
	UE_DEFINE_GAMEPLAY_TAG(Player_GA_Shop_ShopLock, "Player.GA.Shop.ShopLock")
}