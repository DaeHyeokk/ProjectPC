// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Unit/EffectSpec/PCEffectSpec_BasicAttackManaGain.h"

#include "AbilitySystemComponent.h"
#include "Character/Unit/PCBaseUnitCharacter.h"


UPCEffectSpec_BasicAttackManaGain::UPCEffectSpec_BasicAttackManaGain()
{
	TargetGroup = EEffectTargetGroup::Self;
	EffectClassTag = GameplayEffectTags::GE_Class_Unit_ManaChange;
	EffectCallerTag = GameplayEffectTags::GE_Caller_Stat_CurrentMana;
}

FActiveGameplayEffectHandle UPCEffectSpec_BasicAttackManaGain::ApplyEffectImpl(UAbilitySystemComponent* SourceASC,
                                                                               const AActor* Target, int32 EffectLevel)
{
	FActiveGameplayEffectHandle OutHandle;
	
	if (!SourceASC || !Target || !Target->HasAuthority())
		return OutHandle;
	
	if (!IsTargetEligibleByGroup(SourceASC->GetAvatarActor(), Target))
		return OutHandle;

	const TSubclassOf<UGameplayEffect> GEClass = ResolveGEClass(SourceASC->GetWorld());
	if (!GEClass)
		return OutHandle;
	
	const FGameplayEffectContextHandle Ctx = SourceASC->MakeEffectContext();
	const FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(GEClass, 1.f, Ctx);
	if (!SpecHandle.IsValid())
		return OutHandle;

	const APCBaseUnitCharacter* Unit = Cast<APCBaseUnitCharacter>(Target);
	int32 UnitLevel = Unit ? Unit->GetUnitLevel() : 1;
	float ManaGainValue = (UnitLevel <= 1) ? static_cast<float>(FMath::RandRange(6, 10)) : 10.f;

	// 마법사 시너지 버프가 활성화 됐을 경우 마나회복량 2배
	if (SourceASC->HasMatchingGameplayTag(UnitGameplayTags::Unit_Buff_Synergy_Mage_DoubleManaGain))
	{
		ManaGainValue *= 2.f;
	}
	
	SpecHandle.Data->SetSetByCallerMagnitude(EffectCallerTag, ManaGainValue);
	
	OutHandle = SourceASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());

	return OutHandle;
}
