// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Unit/EffectSpec/PCEffectSpec_Damage.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystem/Unit/AttributeSet/PCUnitAttributeSet.h"

UPCEffectSpec_Damage::UPCEffectSpec_Damage()
{
	EffectClassTag = GameplayEffectTags::GE_Class_Unit_Damage;
	EffectCallerTag = GameplayEffectTags::GE_Caller_Damage;
	TargetGroup = EEffectTargetGroup::Hostile;
}

FActiveGameplayEffectHandle UPCEffectSpec_Damage::ApplyEffectImpl(UAbilitySystemComponent* SourceASC, const AActor* Target, int32 EffectLevel)
{
	FActiveGameplayEffectHandle OutHandle;
	
	if (!SourceASC || !Target || !Target->HasAuthority())
		return OutHandle;
	
	if (!IsTargetEligibleByGroup(SourceASC->GetAvatarActor(), Target))
		return OutHandle;
	
	float BaseDamage;
	if (bUseDamageOverride)
	{
		BaseDamage = DamageOverride;
		bUseDamageOverride = false;
	}
	else if (bUseDamageAttribute)
	{
		BaseDamage = SourceASC->GetNumericAttribute(DamageAttribute);
	}
	else
	{
		BaseDamage = FixedDamage;
	}
	
	if (BaseDamage <= 0.f)
		return OutHandle;

	UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Target);
	if (!TargetASC)
		return OutHandle;
	
	const TSubclassOf<UGameplayEffect> GEClass = ResolveGEClass(SourceASC->GetWorld());
	if (!GEClass)
		return OutHandle;
	
	const int32 Level = (EffectLevel > 0) ? EffectLevel : DefaultLevel;
	const FGameplayEffectContextHandle Ctx = SourceASC->MakeEffectContext();
	const FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(GEClass, Level, Ctx);
	if (!SpecHandle.IsValid())
		return OutHandle;
	
	SpecHandle.Data->SetSetByCallerMagnitude(EffectCallerTag, BaseDamage);
	if (DamageType.IsValid())
		SpecHandle.Data->AddDynamicAssetTag(DamageType);
	if (AttackType.IsValid())
		SpecHandle.Data->AddDynamicAssetTag(AttackType);
	if (bNoCrit)
		SpecHandle.Data->AddDynamicAssetTag(NoCritTag);
	if (bNoVamp)
		SpecHandle.Data->AddDynamicAssetTag(NoVampTag);
	if (bNoManaGain)
		SpecHandle.Data->AddDynamicAssetTag(NoManaGainTag);
	if (bNoSendHitEvent)
		SpecHandle.Data->AddDynamicAssetTag(NoSendHitEventTag);
	if (bNoSendAppliedDamageEvent)
		SpecHandle.Data->AddDynamicAssetTag(NoSendDamageAppliedEventTag);

	if (SourceASC->HasMatchingGameplayTag(UnitGameplayTags::Unit_Buff_Synergy_Darkness_TrueDamage))
		SpecHandle.Data->AddDynamicAssetTag(UnitGameplayTags::Unit_DamageType_TrueDamage);
	
	OutHandle = SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
	
	return OutHandle;
}
