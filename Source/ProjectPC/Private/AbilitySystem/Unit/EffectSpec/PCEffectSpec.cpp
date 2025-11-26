// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Unit/EffectSpec/PCEffectSpec.h"

#include "AbilitySystemComponent.h"
#include "GenericTeamAgentInterface.h"

FActiveGameplayEffectHandle UPCEffectSpec::ApplyEffect(UAbilitySystemComponent* SourceASC, const AActor* Target, int32 EffectLevel)
{
	return ApplyEffectImpl(SourceASC, Target, EffectLevel);
}

FActiveGameplayEffectHandle UPCEffectSpec::ApplyEffectSelf(UAbilitySystemComponent* ASC, int32 EffectLevel)
{
	const AActor* Avatar = ASC ? ASC->GetAvatarActor() : nullptr;
	return ApplyEffect(ASC, Avatar, EffectLevel);
}

bool UPCEffectSpec::IsTargetEligibleByGroup(const AActor* Source, const AActor* Target) const
{
	if (!Source || !Target) return false;
	if (TargetGroup == EEffectTargetGroup::Self) return Source == Target;
	if (TargetGroup == EEffectTargetGroup::All) return true;

	const ETeamAttitude::Type Attitude = FGenericTeamId::GetAttitude(Source, Target);
	if (TargetGroup == EEffectTargetGroup::Ally)    return Attitude == ETeamAttitude::Friendly; //&& Source != Target;
	if (TargetGroup == EEffectTargetGroup::Hostile) return Attitude == ETeamAttitude::Hostile;
	return false;
}

void UPCEffectSpec::ApplyDurationOptions(FGameplayEffectSpec& Spec) const
{
	if (bUseDurationSetByCaller && Spec.Def->DurationPolicy == EGameplayEffectDurationType::HasDuration)
	{
		Spec.SetSetByCallerMagnitude(DurationCallerTag, DurationByCallerSeconds);
	}
}
