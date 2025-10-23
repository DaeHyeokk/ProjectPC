// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Unit/EffectSpec/PCEffectSpec_ManaRegen.h"

#include "BaseGameplayTags.h"
#include "AbilitySystem/Unit/AttributeSet/PCHeroUnitAttributeSet.h"


UPCEffectSpec_ManaRegen::UPCEffectSpec_ManaRegen()
{
	ManaRegenAttr = UPCHeroUnitAttributeSet::GetManaRegenAttribute();
	EffectClassTag = GameplayEffectTags::GE_Class_Unit_ManaChange;
	EffectCallerTag = GameplayEffectTags::GE_Caller_Stat_CurrentMana;
	TargetGroup = EEffectTargetGroup::Self;
}

FActiveGameplayEffectHandle UPCEffectSpec_ManaRegen::ApplyEffectImpl(UAbilitySystemComponent* SourceASC,
	const AActor* Target, int32 EffectLevel)
{
	FActiveGameplayEffectHandle OutHandle;
	
	if (!SourceASC || !Target || !Target->HasAuthority())
		return OutHandle;
	
	if (!IsTargetEligibleByGroup(SourceASC->GetAvatarActor(), Target))
		return OutHandle;

	const float ManaRegenValue = SourceASC->GetNumericAttribute(ManaRegenAttr);
	if (ManaRegenValue <= 0.f)
		return OutHandle;
	
	const TSubclassOf<UGameplayEffect> GEClass = ResolveGEClass(SourceASC->GetWorld());
	if (!GEClass)
		return OutHandle;
	
	const int32 Level = (EffectLevel > 0) ? EffectLevel : DefaultLevel;
	const FGameplayEffectContextHandle Ctx = SourceASC->MakeEffectContext();
	const FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(GEClass, Level, Ctx);
	if (!SpecHandle.IsValid())
		return OutHandle;

	SpecHandle.Data->SetSetByCallerMagnitude(EffectCallerTag, ManaRegenValue);
	
	SourceASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());

	return OutHandle;
}
