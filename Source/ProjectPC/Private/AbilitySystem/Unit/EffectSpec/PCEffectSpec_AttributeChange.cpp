// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Unit/EffectSpec/PCEffectSpec_AttributeChange.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"

FActiveGameplayEffectHandle UPCEffectSpec_AttributeChange::ApplyEffectImpl(UAbilitySystemComponent* SourceASC, const AActor* Target,
                                                                            int32 EffectLevel)
{
	FActiveGameplayEffectHandle OutHandle;
	
	if (!SourceASC || !Target || !Target->HasAuthority())
		return OutHandle;
	
	if (!IsTargetEligibleByGroup(SourceASC->GetAvatarActor(), Target))
		return OutHandle;
	
	UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Target);
	if (!TargetASC)
		return OutHandle;

	const TSubclassOf<UGameplayEffect> GEClass = ResolveGEClass(SourceASC->GetWorld());
	if (!GEClass)
		return OutHandle;

	const int32 Level = (EffectLevel > 0) ? EffectLevel : DefaultLevel;
	
	FGameplayEffectContextHandle Ctx = SourceASC->MakeEffectContext();
	FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(GEClass, Level, Ctx);
	if (!SpecHandle.IsValid())
		return OutHandle;

	FGameplayEffectSpec& Spec = *SpecHandle.Data.Get();
	ApplyDurationOptions(Spec);
	
	const float Value = EffectMagnitude.Evaluate(Level);
	SpecHandle.Data->SetSetByCallerMagnitude(EffectCallerTag, Value);

	if (TargetGroup == EEffectTargetGroup::Self)
	{
		OutHandle = SourceASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	}
	else
	{
		OutHandle = SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);	
	}
	
	return OutHandle;
}