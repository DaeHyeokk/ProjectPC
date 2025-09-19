// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Unit/EffectSpec/PCEffectSpec_AttributeChange.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"

void UPCEffectSpec_AttributeChange::ApplyEffectImpl(UAbilitySystemComponent* SourceASC, const AActor* Target,
	int32 EffectLevel)
{
	if (!SourceASC || !Target || !Target->HasAuthority())
		return;
	
	if (!IsTargetEligibleByGroup(SourceASC->GetAvatarActor(), Target))
		return;
	
	UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Target);
	if (!TargetASC)
		return;

	const TSubclassOf<UGameplayEffect> GEClass = ResolveGEClass(SourceASC->GetWorld());
	if (!GEClass)
		return;

	const int32 Level = (EffectLevel > 0) ? EffectLevel : DefaultLevel;
	
	FGameplayEffectContextHandle Ctx = SourceASC->MakeEffectContext();
	FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(GEClass, Level, Ctx);
	if (!SpecHandle.IsValid())
		return;

	const float FinalMag = Magnitude.Evaluate(Level);
	
	SpecHandle.Data->SetSetByCallerMagnitude(EffectCallerTag, FinalMag);

	if (TargetGroup == EEffectTargetGroup::Self)
	{
		SourceASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	}
	else
	{
		SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);	
	}
}
