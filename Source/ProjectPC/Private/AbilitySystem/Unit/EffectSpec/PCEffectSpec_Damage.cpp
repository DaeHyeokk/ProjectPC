// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Unit/EffectSpec/PCEffectSpec_Damage.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"


void UPCEffectSpec_Damage::ApplyEffect(UAbilitySystemComponent* SourceASC, AActor* Context)
{
//	Super::ApplyEffect(SourceASC, Context);

	if (!SourceASC)
		return;
	
	const float BaseDamage = SourceASC->GetNumericAttribute(DamageAttribute);
	if (BaseDamage <= 0.f)
		return;

	UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Context);
	if (!TargetASC)
		return;

	TSubclassOf<UGameplayEffect> GEClass = ResolveGEClass(SourceASC->GetWorld());
	if (!GEClass)
		return;
	
	FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(GEClass, 1.f, SourceASC->MakeEffectContext());
	if (!SpecHandle.IsValid())
		return;
	
	SpecHandle.Data->SetSetByCallerMagnitude(EffectCallerTag, BaseDamage);

	if (DamageType.IsValid())
		SpecHandle.Data->AddDynamicAssetTag(DamageType);

	SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
}
