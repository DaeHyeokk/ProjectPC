// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseGameplayTags.h"

namespace GameStateTags
{
	UE_DEFINE_GAMEPLAY_TAG(Game_State, "Game.State")	
	UE_DEFINE_GAMEPLAY_TAG(Game_State_NonCombat, "Game.State.NonCombat")	
	UE_DEFINE_GAMEPLAY_TAG(Game_State_Combat, "Game.State.Combat")	
	UE_DEFINE_GAMEPLAY_TAG(Game_State_Combat_Preparation, "Game.State.Combat.Preparation")
	UE_DEFINE_GAMEPLAY_TAG(Game_State_Combat_Preparation_Creep, "Game.State.Combat.Preparation.Creep")
	UE_DEFINE_GAMEPLAY_TAG(Game_State_Combat_Active, "Game.State.Combat.Active")
	UE_DEFINE_GAMEPLAY_TAG(Game_State_Combat_Active_Creep, "Game.State.Combat.Active.Creep")	
	UE_DEFINE_GAMEPLAY_TAG(Game_State_Combat_End, "Game.State.Combat.End")	
}

namespace UnitGameplayTags
{
	// ==== Unit Types ====
	UE_DEFINE_GAMEPLAY_TAG(Unit_Type_Hero, "Unit.Type.Hero")
	UE_DEFINE_GAMEPLAY_TAG(Unit_Type_Hero_Sparrow, "Unit.Type.Hero.Sparrow")
	UE_DEFINE_GAMEPLAY_TAG(Unit_Type_Hero_Raven, "Unit.Type.Hero.Raven")
	UE_DEFINE_GAMEPLAY_TAG(Unit_Type_Hero_Drongo, "Unit.Type.Hero.Drongo")
	UE_DEFINE_GAMEPLAY_TAG(Unit_Type_Hero_Greystone, "Unit.Type.Hero.Greystone")
	UE_DEFINE_GAMEPLAY_TAG(Unit_Type_Hero_Yin, "Unit.Type.Hero.Yin")
	
	UE_DEFINE_GAMEPLAY_TAG(Unit_Type_Hero_Revenant, "Unit.Type.Hero.Revenant")
	UE_DEFINE_GAMEPLAY_TAG(Unit_Type_Hero_TwinBlast, "Unit.Type.Hero.TwinBlast")
	UE_DEFINE_GAMEPLAY_TAG(Unit_Type_Hero_Murdock, "Unit.Type.Hero.Murdock")
	UE_DEFINE_GAMEPLAY_TAG(Unit_Type_Hero_Zinx, "Unit.Type.Hero.Zinx")
	UE_DEFINE_GAMEPLAY_TAG(Unit_Type_Creep, "Unit.Type.Creep")
	UE_DEFINE_GAMEPLAY_TAG(Unit_Type_Creep_MinionLv1, "Unit.Type.Creep.MinionLv1")

	// ==== Unit Damage Types
	UE_DEFINE_GAMEPLAY_TAG(Unit_DamageType_Physical, "Unit.DamageType.Physical")
	UE_DEFINE_GAMEPLAY_TAG(Unit_DamageType_Magic, "Unit.DamageType.Magic")

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
	UE_DEFINE_GAMEPLAY_TAG(Unit_Action, "Unit.Action")
	UE_DEFINE_GAMEPLAY_TAG(Unit_Action_Attack, "Unit.Action.Attack")
	UE_DEFINE_GAMEPLAY_TAG(Unit_Action_Attack_Basic, "Unit.Action.Attack.Basic")
	UE_DEFINE_GAMEPLAY_TAG(Unit_Action_Attack_Ultimate, "Unit.Action.Attack.Ultimate")

	// ==== Unit State Tags ====
	UE_DEFINE_GAMEPLAY_TAG(Unit_State_Combat_Alive, "Unit.State.Combat.Alive")
	UE_DEFINE_GAMEPLAY_TAG(Unit_State_Combat_Dead, "Unit.State.Combat.Dead")
	UE_DEFINE_GAMEPLAY_TAG(Unit_State_Combat_Attacking, "Unit.State.Combat.Attacking")
	
	// ==== Unit Cooldown Tags ====
	UE_DEFINE_GAMEPLAY_TAG(Unit_Cooldown_BasicAttack, "Unit.Cooldown.BasicAttack")
	
	// ==== Unit Anim Montage Tags ====
	UE_DEFINE_GAMEPLAY_TAG(Unit_Montage_LevelStart, "Unit.Montage.LevelStart")
	UE_DEFINE_GAMEPLAY_TAG(Unit_Montage_Attack_Basic, "Unit.Montage.Attack.Basic")
	UE_DEFINE_GAMEPLAY_TAG(Unit_Montage_Attack_Ultimate, "Unit.Montage.Attack.Ultimate")
	UE_DEFINE_GAMEPLAY_TAG(Unit_Montage_Death, "Unit.Montage.Death")

	// ==== Unit GameplayEvent Tags ====
	UE_DEFINE_GAMEPLAY_TAG(Unit_Event_SpawnProjectileSucceed, "Unit.Event.SpawnProjectileSucceed")
	UE_DEFINE_GAMEPLAY_TAG(Unit_Event_HitSucceed, "Unit.Event.HitSucceed")
	UE_DEFINE_GAMEPLAY_TAG(Unit_Event_OnHit, "Unit.Event.OnHit")
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
	UE_DEFINE_GAMEPLAY_TAG(GE_Class_HealthChange_Instant, "GE.Class.HealthChange.Instant")
	UE_DEFINE_GAMEPLAY_TAG(GE_Class_ManaChange_Instant, "GE.Class.ManaChange.Instant")
	UE_DEFINE_GAMEPLAY_TAG(GE_Class_Unit_Damage_Instant, "GE.Class.Unit.Damage.Instant")
	
	UE_DEFINE_GAMEPLAY_TAG(GE_Class_Cooldown_BasicAttack, "GE.Class.Cooldown.BasicAttack")

	UE_DEFINE_GAMEPLAY_TAG(GE_Caller_Damage, "GE.Caller.Damage")
	
	UE_DEFINE_GAMEPLAY_TAG(GE_Caller_Stat_CurrentHealth, "GE.Caller.Stat.CurrentHealth")
	UE_DEFINE_GAMEPLAY_TAG(GE_Caller_Stat_MaxHealth, "GE.Caller.Stat.MaxHealth")
	UE_DEFINE_GAMEPLAY_TAG(GE_Caller_Stat_MaxMana, "GE.Caller.Stat.MaxMana")
	UE_DEFINE_GAMEPLAY_TAG(GE_Caller_Stat_CurrentMana, "GE.Caller.Stat.CurrentMana")
	UE_DEFINE_GAMEPLAY_TAG(GE_Caller_Stat_BaseDamage, "GE.Caller.Stat.BaseDamage")
	UE_DEFINE_GAMEPLAY_TAG(GE_Caller_Stat_PhysicalDamageMultiplier, "GE.Caller.Stat.PhysicalDamageMultiplier")
	UE_DEFINE_GAMEPLAY_TAG(GE_Caller_Stat_MagicDamageMultiplier, "GE.Caller.Stat.MagicDamageMultiplier")
	UE_DEFINE_GAMEPLAY_TAG(GE_Caller_Stat_AttackSpeed, "GE.Caller.Stat.AttackSpeed")
	UE_DEFINE_GAMEPLAY_TAG(GE_Caller_Stat_PhysicalDefense, "GE.Caller.Stat.PhysicalDefense")
	UE_DEFINE_GAMEPLAY_TAG(GE_Caller_Stat_MagicDefense, "GE.Caller.Stat.MagicDefense")
	UE_DEFINE_GAMEPLAY_TAG(GE_Caller_Stat_CritChance, "GE.Caller.Stat.CritChance")
	UE_DEFINE_GAMEPLAY_TAG(GE_Caller_Stat_LifeSteal, "GE.Caller.Stat.LifeSteal")
	UE_DEFINE_GAMEPLAY_TAG(GE_Caller_Stat_SpellVamp, "GE.Caller.Stat.SpellVamp")
	
	UE_DEFINE_GAMEPLAY_TAG(GE_Caller_Cooldown_BasicAttack, "GE.Caller.Cooldown.BasicAttack")
}

namespace GameplayCueTags
{
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Unit_LevelUp, "GameplayCue.Unit.LevelUp")
}

namespace PlayerGameplayTags
{
	// ==== Player Type Tags ====
	UE_DEFINE_GAMEPLAY_TAG(Player_Type_LittleDragon, "Player.Type.LittleDragon")
	
	// ==== Player Stat Tags ====
	UE_DEFINE_GAMEPLAY_TAG(Player_Stat_PlayerLevel, "Player.Stat.PlayerLevel")
	UE_DEFINE_GAMEPLAY_TAG(Player_Stat_PlayerXP, "Player.Stat.PlayerXP")
	UE_DEFINE_GAMEPLAY_TAG(Player_Stat_PlayerGold, "Player.Stat.PlayerGold")
	UE_DEFINE_GAMEPLAY_TAG(Player_Stat_PlayerHP, "Player.Stat.PlayerHP")

	// ==== Player State Tags ====
	UE_DEFINE_GAMEPLAY_TAG(Player_State_Normal, "Player.State.Normal")
	UE_DEFINE_GAMEPLAY_TAG(Player_State_Carousel, "Player.State.Carousel")
	UE_DEFINE_GAMEPLAY_TAG(Player_State_Dead, "Player.State.Dead")

	// ==== Player Shop Ability Tags ====
	UE_DEFINE_GAMEPLAY_TAG(Player_GA_Shop_BuyXP, "Player.GA.Shop.BuyXP")
	UE_DEFINE_GAMEPLAY_TAG(Player_GA_Shop_BuyUnit, "Player.GA.Shop.BuyUnit")
	UE_DEFINE_GAMEPLAY_TAG(Player_GA_Shop_SellUnit, "Player.GA.Shop.SellUnit")
	UE_DEFINE_GAMEPLAY_TAG(Player_GA_Shop_ShopRefresh, "Player.GA.Shop.ShopRefresh")
	UE_DEFINE_GAMEPLAY_TAG(Player_GA_Shop_ShopLock, "Player.GA.Shop.ShopLock")

	// ==== Player Action Ability Tags ====
	UE_DEFINE_GAMEPLAY_TAG(Player_Action_Attack_Basic, "Player.Action.Attack.Basic")

	// ==== Player Damage Event Tags ====
	UE_DEFINE_GAMEPLAY_TAG(Player_Event_Damage, "Player.Event.Damage")
	UE_DEFINE_GAMEPLAY_TAG(Player_Event_ProjectileHit, "Player.Event.ProjectileHit")
	
}

namespace GameRoundTags
{
	UE_DEFINE_GAMEPLAY_TAG(GameRound_Start, "GameRound.Start")
	UE_DEFINE_GAMEPLAY_TAG(GameRound_SetUp, "GameRound.SetUp")
	UE_DEFINE_GAMEPLAY_TAG(GameRound_PvP, "GameRound.PvP")
	UE_DEFINE_GAMEPLAY_TAG(GameRound_PvE, "GameRound.PvE")
	UE_DEFINE_GAMEPLAY_TAG(GameRound_PvE_MinionsLv1, "GameRound.PvE.MinionsLv1")
	UE_DEFINE_GAMEPLAY_TAG(GameRound_PvE_MinionsLv2, "GameRound.PvE.MinionsLv2")
	UE_DEFINE_GAMEPLAY_TAG(GameRound_PvE_MinionsLv3, "GameRound.PvE.MinionsLv3")
	UE_DEFINE_GAMEPLAY_TAG(GameRound_PvE_MinionsLv4, "GameRound.PvE.MinionsLv4")
	UE_DEFINE_GAMEPLAY_TAG(GameRound_Carousel, "GameRound.Carousel")
}

namespace ItemTags
{
	// ==== Base Item Tags ====
	UE_DEFINE_GAMEPLAY_TAG(Item_Type_Base_BFSword, "Item.Type.Base.BFSword")
	UE_DEFINE_GAMEPLAY_TAG(Item_Type_Base_ChainVest, "Item.Type.Base.ChainVest")
	UE_DEFINE_GAMEPLAY_TAG(Item_Type_Base_GiantsBelt, "Item.Type.Base.GiantsBelt")
	UE_DEFINE_GAMEPLAY_TAG(Item_Type_Base_LargeRod, "Item.Type.Base.LargeRod")
	UE_DEFINE_GAMEPLAY_TAG(Item_Type_Base_NegatronCloak, "Item.Type.Base.NegatronCloak")
	UE_DEFINE_GAMEPLAY_TAG(Item_Type_Base_RecurveBow, "Item.Type.Base.RecurveBow")
	UE_DEFINE_GAMEPLAY_TAG(Item_Type_Base_SparringGloves, "Item.Type.Base.SparringGloves")

	// ==== Advanced Item Tags ====
	UE_DEFINE_GAMEPLAY_TAG(Item_Type_Advanced_BloodThirster, "Item.Type.Advanced.BloodThirster")
	UE_DEFINE_GAMEPLAY_TAG(Item_Type_Advanced_BrambleVest, "Item.Type.Advanced.BrambleVest")
	UE_DEFINE_GAMEPLAY_TAG(Item_Type_Advanced_CrownGuard, "Item.Type.Advanced.CrownGuard")
	UE_DEFINE_GAMEPLAY_TAG(Item_Type_Advanced_DeathBlade, "Item.Type.Advanced.DeathBlade")
	UE_DEFINE_GAMEPLAY_TAG(Item_Type_Advanced_DeathCap, "Item.Type.Advanced.DeathCap")
	UE_DEFINE_GAMEPLAY_TAG(Item_Type_Advanced_DragonsClaw, "Item.Type.Advanced.DragonsClaw")
	UE_DEFINE_GAMEPLAY_TAG(Item_Type_Advanced_EdgeofNight, "Item.Type.Advanced.EdgeofNight")
	UE_DEFINE_GAMEPLAY_TAG(Item_Type_Advanced_EvenShroud, "Item.Type.Advanced.EvenShroud")
	UE_DEFINE_GAMEPLAY_TAG(Item_Type_Advanced_GargoyleStoneplate, "Item.Type.Advanced.GargoyleStoneplate")
	UE_DEFINE_GAMEPLAY_TAG(Item_Type_Advanced_GiantSlayer, "Item.Type.Advanced.GiantSlayer")
	UE_DEFINE_GAMEPLAY_TAG(Item_Type_Advanced_GuinsoosRageblade, "Item.Type.Advanced.GuinsoosRageblade")
	UE_DEFINE_GAMEPLAY_TAG(Item_Type_Advanced_HextechGunblade, "Item.Type.Advanced.HextechGunblade")
	UE_DEFINE_GAMEPLAY_TAG(Item_Type_Advanced_InfintiyEdge, "Item.Type.Advanced.InfintiyEdge")
	UE_DEFINE_GAMEPLAY_TAG(Item_Type_Advanced_IonicSpark, "Item.Type.Advanced.IonicSpark")
	UE_DEFINE_GAMEPLAY_TAG(Item_Type_Advanced_JeweledGauntlet, "Item.Type.Advanced.JeweledGauntlet")
	UE_DEFINE_GAMEPLAY_TAG(Item_Type_Advanced_KrakensFury, "Item.Type.Advanced.KrakensFury")
	UE_DEFINE_GAMEPLAY_TAG(Item_Type_Advanced_LastWhisper, "Item.Type.Advanced.LastWhisper")
	UE_DEFINE_GAMEPLAY_TAG(Item_Type_Advanced_Morellonomicon, "Item.Type.Advanced.Morellonomicon")
	UE_DEFINE_GAMEPLAY_TAG(Item_Type_Advanced_NashorsTooth, "Item.Type.Advanced.NashorsTooth")
	UE_DEFINE_GAMEPLAY_TAG(Item_Type_Advanced_QuickSilver, "Item.Type.Advanced.QuickSilver")
	UE_DEFINE_GAMEPLAY_TAG(Item_Type_Advanced_RedBuff, "Item.Type.Advanced.RedBuff")
	UE_DEFINE_GAMEPLAY_TAG(Item_Type_Advanced_SteadfastHeart, "Item.Type.Advanced.SteadfastHeart")
	UE_DEFINE_GAMEPLAY_TAG(Item_Type_Advanced_SteraksGage, "Item.Type.Advanced.SteraksGage")
	UE_DEFINE_GAMEPLAY_TAG(Item_Type_Advanced_StrikersFrail, "Item.Type.Advanced.StrikersFrail")
	UE_DEFINE_GAMEPLAY_TAG(Item_Type_Advanced_SunfireCape, "Item.Type.Advanced.SunfireCape")
	UE_DEFINE_GAMEPLAY_TAG(Item_Type_Advanced_ThiefsGloves, "Item.Type.Advanced.ThiefsGloves")
	UE_DEFINE_GAMEPLAY_TAG(Item_Type_Advanced_TitansResolve, "Item.Type.Advanced.TitansResolve")
	UE_DEFINE_GAMEPLAY_TAG(Item_Type_Advanced_WarmogsArmor, "Item.Type.Advanced.WarmogsArmor")
}