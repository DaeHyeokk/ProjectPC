// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Unit/EffectSpec/PCEffectSpec_Static.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"


FActiveGameplayEffectHandle UPCEffectSpec_Static::ApplyEffectImpl(UAbilitySystemComponent* SourceASC,
                                                                   const AActor* Target, int32 EffectLevel)
{
	FActiveGameplayEffectHandle Result;

	if (!SourceASC || !Target || !Target->HasAuthority())
		return Result;

	if (!IsTargetEligibleByGroup(SourceASC->GetAvatarActor(), Target))
		return Result;

	UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Target);
	if (!TargetASC || !GEClass)
		return Result;

	const int32 Level = (EffectLevel > 0) ? EffectLevel : DefaultLevel;

	FGameplayEffectContextHandle Ctx = SourceASC->MakeEffectContext();
	FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(GEClass, Level, Ctx);
	if (!SpecHandle.IsValid())
		return Result;

	FGameplayEffectSpec& Spec = *SpecHandle.Data.Get();
	ApplyDurationOptions(Spec);
	
	if (TargetGroup == EEffectTargetGroup::Self)
	{
		Result = SourceASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	}
	else
	{
		Result = SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
	}

	return Result;
}
