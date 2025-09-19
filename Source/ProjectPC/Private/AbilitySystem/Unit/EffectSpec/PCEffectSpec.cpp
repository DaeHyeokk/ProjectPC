// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Unit/EffectSpec/PCEffectSpec.h"

#include "AbilitySystemComponent.h"
#include "GenericTeamAgentInterface.h"
#include "GameFramework/WorldSubsystem/PCUnitGERegistrySubsystem.h"

void UPCEffectSpec::ApplyEffect(UAbilitySystemComponent* SourceASC, const AActor* Target)
{
	ApplyEffectImpl(SourceASC, Target, -1);
}

void UPCEffectSpec::ApplyEffect(UAbilitySystemComponent* SourceASC, const AActor* Target, int32 EffectLevel)
{
	ApplyEffectImpl(SourceASC, Target, EffectLevel);
}

TSubclassOf<UGameplayEffect> UPCEffectSpec::ResolveGEClass(const UWorld* World)
{
	if (CachedGEClass)
		return CachedGEClass;

	if (!World || !EffectClassTag.IsValid())
		return nullptr;
	
	if (auto* GERegistrySubsystem = World->GetSubsystem<UPCUnitGERegistrySubsystem>())
	{
		if (TSubclassOf<UGameplayEffect> GEClass = GERegistrySubsystem->GetGEClass(EffectClassTag))
		{
			CachedGEClass = GEClass;
			return GEClass;
		}
	}

	return nullptr;
}

bool UPCEffectSpec::IsTargetEligibleByGroup(const AActor* Source, const AActor* Target) const
{
	if (!Source || !Target) return false;
	if (TargetGroup == EEffectTargetGroup::Self) return Source == Target;
	if (TargetGroup == EEffectTargetGroup::All) return true;

	const ETeamAttitude::Type Attitude = FGenericTeamId::GetAttitude(Source, Target);
	if (TargetGroup == EEffectTargetGroup::Ally)    return Attitude == ETeamAttitude::Friendly && Source != Target;
	if (TargetGroup == EEffectTargetGroup::Hostile) return Attitude == ETeamAttitude::Hostile;
	return false;
}
