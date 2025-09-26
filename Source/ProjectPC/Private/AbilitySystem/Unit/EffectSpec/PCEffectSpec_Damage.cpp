// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Unit/EffectSpec/PCEffectSpec_Damage.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"

void UPCEffectSpec_Damage::ApplyEffectImpl(UAbilitySystemComponent* SourceASC, const AActor* Target, int32 EffectLevel)
{
	if (!SourceASC || !Target || !Target->HasAuthority())
		return;
	
	if (!IsTargetEligibleByGroup(SourceASC->GetAvatarActor(), Target))
		return;
	
	const float BaseDamage = SourceASC->GetNumericAttribute(DamageAttribute);
	if (BaseDamage <= 0.f)
		return;

	UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Target);
	if (!TargetASC)
		return;

	const TSubclassOf<UGameplayEffect> GEClass = ResolveGEClass(SourceASC->GetWorld());
	if (!GEClass)
		return;
	
	const int32 Level = (EffectLevel > 0) ? EffectLevel : DefaultLevel;
	const FGameplayEffectContextHandle Ctx = SourceASC->MakeEffectContext();
	const FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(GEClass, Level, Ctx);
	if (!SpecHandle.IsValid())
		return;
	
	SpecHandle.Data->SetSetByCallerMagnitude(EffectCallerTag, BaseDamage);
	if (DamageType.IsValid())
		SpecHandle.Data->AddDynamicAssetTag(DamageType);

	SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
	FGameplayEventData Data;
	Data.EventTag = HitEventTag;
	Data.Instigator = SourceASC->GetAvatarActor();
	Data.Target = Target;
	TargetASC->HandleGameplayEvent(HitEventTag, &Data);
}
