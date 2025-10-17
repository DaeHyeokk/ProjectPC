// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Unit/EffectSpec/PCEffectSpec_Damage.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystem/Unit/AttributeSet/PCUnitAttributeSet.h"

UPCEffectSpec_Damage::UPCEffectSpec_Damage()
{
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
	
	float Evasion = TargetASC->GetNumericAttribute(UPCUnitAttributeSet::GetEvasionChanceAttribute());
	Evasion *= 0.01f;
	if (Evasion > 0.f && FMath::FRand() < Evasion)
	{
		// 데미지 입는 대상 회피 성공, 바로 리턴 
		return OutHandle;
	}
	
	const TSubclassOf<UGameplayEffect> GEClass = ResolveGEClass(SourceASC->GetWorld());
	if (!GEClass)
		return OutHandle;
	
	const int32 Level = (EffectLevel > 0) ? EffectLevel : DefaultLevel;
	const FGameplayEffectContextHandle Ctx = SourceASC->MakeEffectContext();
	const FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(GEClass, Level, Ctx);
	if (!SpecHandle.IsValid())
		return OutHandle;

	// 공격 적중에 성공할 경우 이벤트 발생 (데미지 적용 전에 호출하는 이벤트)
	if (!bNoSendHitEvent)
	{
		FGameplayTag HitSucceedEventTag;
		switch (AttackType)
		{
		case EAttackType::Basic:
			HitSucceedEventTag = BasicHitSucceedEventTag;
			break;

		case EAttackType::Ultimate:
			HitSucceedEventTag = UltimateHitSucceedEventTag;
			break;
			
		case EAttackType::BonusDamage:
			HitSucceedEventTag = BonusDmgHitSucceedEventTag;
			break;

		case EAttackType::None:
			break;
		}

		if (HitSucceedEventTag.IsValid())
		{
			FGameplayEventData HitSucceedData;
			HitSucceedData.EventTag = HitSucceedEventTag;
			HitSucceedData.Instigator = SourceASC->GetAvatarActor();
			HitSucceedData.Target = Target;
			SourceASC->HandleGameplayEvent(HitSucceedEventTag, &HitSucceedData);
		}
		
		FGameplayEventData OnHitData;
		OnHitData.EventTag = OnHitEventTag;
		OnHitData.Instigator = SourceASC->GetAvatarActor();
		OnHitData.Target = Target;
		TargetASC->HandleGameplayEvent(OnHitEventTag, &OnHitData);
	}
	
	SpecHandle.Data->SetSetByCallerMagnitude(EffectCallerTag, BaseDamage);
	if (DamageType.IsValid())
		SpecHandle.Data->AddDynamicAssetTag(DamageType);
	if (bNoCrit)
		SpecHandle.Data->AddDynamicAssetTag(NoCritTag);
	if (bNoVamp)
		SpecHandle.Data->AddDynamicAssetTag(NoVampTag);
	if (bNoManaGain)
		SpecHandle.Data->AddDynamicAssetTag(NoManaGainTag);
	if (bNoSendAppliedDamageEvent)
		SpecHandle.Data->AddDynamicAssetTag(NoSendDamageAppliedEventTag);

	if (SourceASC->HasMatchingGameplayTag(UnitGameplayTags::Unit_Buff_Synergy_Darkness_TrueDamage))
		SpecHandle.Data->AddDynamicAssetTag(UnitGameplayTags::Unit_DamageType_TrueDamage);
	
	OutHandle = SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
	
	return OutHandle;
}
