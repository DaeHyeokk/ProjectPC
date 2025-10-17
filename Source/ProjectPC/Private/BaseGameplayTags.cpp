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
	UE_DEFINE_GAMEPLAY_TAG(Unit_DamageType_TrueDamage, "Unit.DamageType.TrueDamage")

	// ==== Unit Damage Flags ====
	UE_DEFINE_GAMEPLAY_TAG(Unit_DamageFlag_NoCrit, "Unit.DamageFlag.NoCrit")
	UE_DEFINE_GAMEPLAY_TAG(Unit_DamageFlag_NoVamp, "Unit.DamageFlag.NoVamp")
	UE_DEFINE_GAMEPLAY_TAG(Unit_DamageFlag_NoManaGain, "Unit.DamageFlag.NoManaGain")
	UE_DEFINE_GAMEPLAY_TAG(Unit_DamageFlag_NoSendDamageAppliedEvent, "Unit.DamageFlag.NoSendDamageAppliedEvent")
	
	// ==== Unit Stat Tags ====
	// UE_DEFINE_GAMEPLAY_TAG(Unit_Stat_Shared_MaxHealth, "Unit.Stat.Shared.MaxHealth")
	// UE_DEFINE_GAMEPLAY_TAG(Unit_Stat_Shared_CurrentHealth, "Unit.Stat.Shared.CurrentHealth")
	// UE_DEFINE_GAMEPLAY_TAG(Unit_Stat_Shared_BaseDamage, "Unit.Stat.Shared.BaseDamage")
	// UE_DEFINE_GAMEPLAY_TAG(Unit_Stat_Shared_AttackRange, "Unit.Stat.Shared.AttackRange")
	// UE_DEFINE_GAMEPLAY_TAG(Unit_Stat_Shared_AttackSpeed, "Unit.Stat.Shared.AttackSpeed")
	// UE_DEFINE_GAMEPLAY_TAG(Unit_Stat_Shared_PhysicalDefense, "Unit.Stat.Shared.PhysicalDefense")
	// UE_DEFINE_GAMEPLAY_TAG(Unit_Stat_Shared_MagicDefense, "Unit.Stat.Shared.MagicDefense")
	//
	// UE_DEFINE_GAMEPLAY_TAG(Unit_Stat_Hero_MaxMana, "Unit.Stat.Hero.MaxMana")
	// UE_DEFINE_GAMEPLAY_TAG(Unit_Stat_Hero_CurrentMana, "Unit.Stat.Hero.CurrentMana")
	// UE_DEFINE_GAMEPLAY_TAG(Unit_Stat_Hero_PhysicalDamageMultiplier, "Unit.Stat.Hero.PhysicalDamageMultiplier")
	// UE_DEFINE_GAMEPLAY_TAG(Unit_Stat_Hero_MagicDamageMultiplier, "Unit.Stat.Hero.MagicDamageMultiplier")
	// UE_DEFINE_GAMEPLAY_TAG(Unit_Stat_Hero_CritChance, "Unit.Stat.Hero.CritChance")
	// UE_DEFINE_GAMEPLAY_TAG(Unit_Stat_Hero_LifeSteal, "Unit.Stat.Hero.LifeSteal")
	// UE_DEFINE_GAMEPLAY_TAG(Unit_Stat_Hero_SpellVamp, "Unit.Stat.Hero.SpellVamp")

	// ==== Unit Combat Text Type Tags ====
	UE_DEFINE_GAMEPLAY_TAG(Unit_CombatText_Damage_Physical, "Unit.CombatText.Damage.Physical")
	UE_DEFINE_GAMEPLAY_TAG(Unit_CombatText_Damage_Magic, "Unit.CombatText.Damage.Magic")
	UE_DEFINE_GAMEPLAY_TAG(Unit_CombatText_Damage_TrueDamage, "Unit.CombatText.Damage.TrueDamage")
	UE_DEFINE_GAMEPLAY_TAG(Unit_CombatText_Heal, "Unit.CombatText.Heal")
	
	// ==== Unit Action Ability Tags
	UE_DEFINE_GAMEPLAY_TAG(Unit_Ability, "Unit.Ability")
	UE_DEFINE_GAMEPLAY_TAG(Unit_Ability_Attack, "Unit.Ability.Attack")
	UE_DEFINE_GAMEPLAY_TAG(Unit_Ability_Attack_Basic, "Unit.Ability.Attack.Basic")
	UE_DEFINE_GAMEPLAY_TAG(Unit_Ability_Attack_Ultimate, "Unit.Ability.Attack.Ultimate")
	UE_DEFINE_GAMEPLAY_TAG(Unit_Ability_HitSucceed, "Unit.Ability.HitSucceed")
	UE_DEFINE_GAMEPLAY_TAG(Unit_Ability_PeriodPulse, "Unit.Ability.PeriodPulse")
	UE_DEFINE_GAMEPLAY_TAG(Unit_Ability_Synergy_Cyborg, "Unit.Ability.Synergy.Cyborg")
	UE_DEFINE_GAMEPLAY_TAG(Unit_Ability_Synergy_Guardian, "Unit.Ability.Synergy.Guardian")
	UE_DEFINE_GAMEPLAY_TAG(Unit_Ability_Synergy_Undead, "Unit.Ability.Synergy.Undead")
	UE_DEFINE_GAMEPLAY_TAG(Unit_Ability_Synergy_Demon, "Unit.Ability.Synergy.Demon")
	UE_DEFINE_GAMEPLAY_TAG(Unit_Ability_Synergy_Imperial, "Unit.Ability.Synergy.Imperial")
	UE_DEFINE_GAMEPLAY_TAG(Unit_Ability_Synergy_Beast, "Unit.Ability.Synergy.Beast")
	UE_DEFINE_GAMEPLAY_TAG(Unit_Ability_Synergy_BladeMaster, "Unit.Ability.Synergy.BladeMaster")
	UE_DEFINE_GAMEPLAY_TAG(Unit_Ability_Synergy_Mechanic, "Unit.Ability.Synergy.Mechanic")
	UE_DEFINE_GAMEPLAY_TAG(Unit_Ability_Synergy_Marksman, "Unit.Ability.Synergy.Marksman")

	// ==== Unit State Tags ====
	UE_DEFINE_GAMEPLAY_TAG(Unit_State_Combat_Alive, "Unit.State.Combat.Alive")
	UE_DEFINE_GAMEPLAY_TAG(Unit_State_Combat_Dead, "Unit.State.Combat.Dead")
	UE_DEFINE_GAMEPLAY_TAG(Unit_State_Combat_Attacking, "Unit.State.Combat.Attacking")
	UE_DEFINE_GAMEPLAY_TAG(Unit_State_Combat_Stun, "Unit.State.Combat.Stun")
	
	// ==== Unit Cooldown Tags ====
	UE_DEFINE_GAMEPLAY_TAG(Unit_Cooldown_BasicAttack, "Unit.Cooldown.BasicAttack")
	
	// ==== Unit Anim Montage Tags ====
	UE_DEFINE_GAMEPLAY_TAG(Unit_Montage_LevelStart, "Unit.Montage.LevelStart")
	UE_DEFINE_GAMEPLAY_TAG(Unit_Montage_Attack_Basic, "Unit.Montage.Attack.Basic")
	UE_DEFINE_GAMEPLAY_TAG(Unit_Montage_Attack_Ultimate, "Unit.Montage.Attack.Ultimate")
	UE_DEFINE_GAMEPLAY_TAG(Unit_Montage_Death, "Unit.Montage.Death")

	// ==== Unit GameplayEvent Tags ====
	UE_DEFINE_GAMEPLAY_TAG(Unit_Event_SpawnProjectileSucceed, "Unit.Event.SpawnProjectileSucceed")
	UE_DEFINE_GAMEPLAY_TAG(Unit_Event_AttackSucceed, "Unit.Event.AttackSucceed")
	UE_DEFINE_GAMEPLAY_TAG(Unit_Event_HitSucceed, "Unit.Event.HitSucceed")
	UE_DEFINE_GAMEPLAY_TAG(Unit_Event_HitSucceed_Basic, "Unit.Event.HitSucceed.Basic")
	UE_DEFINE_GAMEPLAY_TAG(Unit_Event_HitSucceed_Ultimate, "Unit.Event.HitSucceed.Ultimate")
	UE_DEFINE_GAMEPLAY_TAG(Unit_Event_HitSucceed_BonusDamage, "Unit.Event.HitSucceed.BonusDamage")
	UE_DEFINE_GAMEPLAY_TAG(Unit_Event_DamageApplied, "Unit.Event.DamageApplied")
	UE_DEFINE_GAMEPLAY_TAG(Unit_Event_OnHit, "Unit.Event.OnHit")

	// ==== Unit Buff Tags ====
	UE_DEFINE_GAMEPLAY_TAG(Unit_Buff_Synergy_Darkness_TrueDamage, "Unit.Buff.Synergy.Darkness.TrueDamage")
}

namespace SynergyGameplayTags
{
	UE_DEFINE_GAMEPLAY_TAG(Synergy, "Synergy")
	
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
	// ==== Instant GE ====
	UE_DEFINE_GAMEPLAY_TAG(GE_Class_Unit_ManaChange, "GE.Class.Unit.ManaChange")
	UE_DEFINE_GAMEPLAY_TAG(GE_Class_Unit_Damage, "GE.Class.Unit.Damage")
	UE_DEFINE_GAMEPLAY_TAG(GE_Class_Unit_Heal, "GE.Class.Unit.Heal")

	// ==== Infinite GE ====
	UE_DEFINE_GAMEPLAY_TAG(GE_Class_Unit_AttributeChange_Add_Infinite_MaxHealth,						"GE.Class.Unit.AttributeChange.Add.Infinite.MaxHealth")
	UE_DEFINE_GAMEPLAY_TAG(GE_Class_Unit_AttributeChange_Add_Infinite_CombatStartMana,					"GE.Class.Unit.AttributeChange.Add.Infinite.CombatStartMana")
	UE_DEFINE_GAMEPLAY_TAG(GE_Class_Unit_AttributeChange_Add_Infinite_BaseDamage,						"GE.Class.Unit.AttributeChange.Add.Infinite.BaseDamage")
	UE_DEFINE_GAMEPLAY_TAG(GE_Class_Unit_AttributeChange_Add_Infinite_PhysicalDamageMultiplier,			"GE.Class.Unit.AttributeChange.Add.Infinite.PhysicalDamageMultiplier")
	UE_DEFINE_GAMEPLAY_TAG(GE_Class_Unit_AttributeChange_Add_Infinite_MagicDamageMultiplier,			"GE.Class.Unit.AttributeChange.Add.Infinite.MagicDamageMultiplier")
	UE_DEFINE_GAMEPLAY_TAG(GE_Class_Unit_AttributeChange_Add_Infinite_DamageMultiplier,					"GE.Class.Unit.AttributeChange.Add.Infinite.DamageMultiplier")
	UE_DEFINE_GAMEPLAY_TAG(GE_Class_Unit_AttributeChange_Add_Infinite_AttackRange,						"GE.Class.Unit.AttributeChange.Add.Infinite.AttackRange")
	UE_DEFINE_GAMEPLAY_TAG(GE_Class_Unit_AttributeChange_Add_Infinite_AttackSpeedIncreaseMultiplier,	"GE.Class.Unit.AttributeChange.Add.Infinite.AttackSpeedIncreaseMultiplier")
	UE_DEFINE_GAMEPLAY_TAG(GE_Class_Unit_AttributeChange_Add_Infinite_AttackSpeedDecreaseMultiplier,	"GE.Class.Unit.AttributeChange.Add.Infinite.AttackSpeedDecreaseMultiplier")
	UE_DEFINE_GAMEPLAY_TAG(GE_Class_Unit_AttributeChange_Add_Infinite_PhysicalDefense,					"GE.Class.Unit.AttributeChange.Add.Infinite.PhysicalDefense")
	UE_DEFINE_GAMEPLAY_TAG(GE_Class_Unit_AttributeChange_Add_Infinite_MagicDefense,						"GE.Class.Unit.AttributeChange.Add.Infinite.MagicDefense")
	UE_DEFINE_GAMEPLAY_TAG(GE_Class_Unit_AttributeChange_Add_Infinite_CritChance,						"GE.Class.Unit.AttributeChange.Add.Infinite.CritChance")
	UE_DEFINE_GAMEPLAY_TAG(GE_Class_Unit_AttributeChange_Add_Infinite_CritMultiplier,					"GE.Class.Unit.AttributeChange.Add.Infinite.CritMultiplier")
	UE_DEFINE_GAMEPLAY_TAG(GE_Class_Unit_AttributeChange_Add_Infinite_LifeSteal,						"GE.Class.Unit.AttributeChange.Add.Infinite.LifeSteal")
	UE_DEFINE_GAMEPLAY_TAG(GE_Class_Unit_AttributeChange_Add_Infinite_FlatDamageBlock,					"GE.Class.Unit.AttributeChange.Add.Infinite.FlatDamageBlock")
	UE_DEFINE_GAMEPLAY_TAG(GE_Class_Unit_AttributeChange_Add_Infinite_EvasionChance,					"GE.Class.Unit.AttributeChange.Add.Infinite.EvasionChance")
	
	// ==== Duration GE ====
	UE_DEFINE_GAMEPLAY_TAG(GE_Class_Cooldown_BasicAttack, "GE.Class.Cooldown.BasicAttack")
	
	UE_DEFINE_GAMEPLAY_TAG(GE_Caller_Damage, "GE.Caller.Damage")
	UE_DEFINE_GAMEPLAY_TAG(GE_Caller_Heal, "GE.Caller.Heal")
	
	UE_DEFINE_GAMEPLAY_TAG(GE_Caller_Stat_CurrentHealth, "GE.Caller.Stat.CurrentHealth")
	UE_DEFINE_GAMEPLAY_TAG(GE_Caller_Stat_MaxHealth, "GE.Caller.Stat.MaxHealth")
	UE_DEFINE_GAMEPLAY_TAG(GE_Caller_Stat_MaxMana, "GE.Caller.Stat.MaxMana")
	UE_DEFINE_GAMEPLAY_TAG(GE_Caller_Stat_CurrentMana, "GE.Caller.Stat.CurrentMana")
	UE_DEFINE_GAMEPLAY_TAG(GE_Caller_Stat_CombatStartMana, "GE.Caller.Stat.CombatStartMana")
	UE_DEFINE_GAMEPLAY_TAG(GE_Caller_Stat_BaseDamage, "GE.Caller.Stat.BaseDamage")
	UE_DEFINE_GAMEPLAY_TAG(GE_Caller_Stat_PhysicalDamageMultiplier, "GE.Caller.Stat.PhysicalDamageMultiplier")
	UE_DEFINE_GAMEPLAY_TAG(GE_Caller_Stat_MagicDamageMultiplier, "GE.Caller.Stat.MagicDamageMultiplier")
	UE_DEFINE_GAMEPLAY_TAG(GE_Caller_Stat_DamageMultiplier, "GE.Caller.Stat.DamageMultiplier")
	UE_DEFINE_GAMEPLAY_TAG(GE_Caller_Stat_AttackRange, "GE.Caller.Stat.AttackRange")
	UE_DEFINE_GAMEPLAY_TAG(GE_Caller_Stat_AttackSpeed, "GE.Caller.Stat.AttackSpeed")
	UE_DEFINE_GAMEPLAY_TAG(GE_Caller_Stat_AttackSpeedIncreaseMultiplier, "GE.Caller.Stat.AttackSpeedIncreaseMultiplier")
	UE_DEFINE_GAMEPLAY_TAG(GE_Caller_Stat_AttackSpeedDecreaseMultiplier, "GE.Caller.Stat.AttackSpeedDecreaseMultiplier")
	UE_DEFINE_GAMEPLAY_TAG(GE_Caller_Stat_PhysicalDefense, "GE.Caller.Stat.PhysicalDefense")
	UE_DEFINE_GAMEPLAY_TAG(GE_Caller_Stat_MagicDefense, "GE.Caller.Stat.MagicDefense")
	UE_DEFINE_GAMEPLAY_TAG(GE_Caller_Stat_CritChance, "GE.Caller.Stat.CritChance")
	UE_DEFINE_GAMEPLAY_TAG(GE_Caller_Stat_CritMultiplier, "GE.Caller.Stat.CritMultiplier")
	UE_DEFINE_GAMEPLAY_TAG(GE_Caller_Stat_LifeSteal, "GE.Caller.Stat.LifeSteal")
	UE_DEFINE_GAMEPLAY_TAG(GE_Caller_Stat_FlatDamageBlock, "GE.Caller.Stat.FlatDamageBlock")
	UE_DEFINE_GAMEPLAY_TAG(GE_Caller_Stat_EvasionChance, "GE.Caller.Stat.EvasionChance")
	
	UE_DEFINE_GAMEPLAY_TAG(GE_Caller_Cooldown_BasicAttack, "GE.Caller.Cooldown.BasicAttack")
	UE_DEFINE_GAMEPLAY_TAG(GE_Caller_Duration, "GE.Caller.Duration")
}

namespace GameplayCueTags
{
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_VFX_Unit_LevelUp, "GameplayCue.VFX.Unit.LevelUp")
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_VFX_Unit_Haste, "GameplayCue.VFX.Unit.Haste")
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_UI_Unit_CombatText, "GameplayCue.UI.Unit.CombatText")
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

	// ==== Player Shop Ability Tags ====
	UE_DEFINE_GAMEPLAY_TAG(Player_GA_Shop_BuyXP, "Player.GA.Shop.BuyXP")
	UE_DEFINE_GAMEPLAY_TAG(Player_GA_Shop_BuyUnit, "Player.GA.Shop.BuyUnit")
	UE_DEFINE_GAMEPLAY_TAG(Player_GA_Shop_SellUnit, "Player.GA.Shop.SellUnit")
	UE_DEFINE_GAMEPLAY_TAG(Player_GA_Shop_ShopRefresh, "Player.GA.Shop.ShopRefresh")
	UE_DEFINE_GAMEPLAY_TAG(Player_GA_Shop_ShopLock, "Player.GA.Shop.ShopLock")

	// ==== Player Damage Evnet Tags ====
	UE_DEFINE_GAMEPLAY_TAG(Player_Event_Damage, "Player.Event.Damage")
	
}