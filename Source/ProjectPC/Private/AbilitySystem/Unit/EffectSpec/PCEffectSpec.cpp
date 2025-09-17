// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Unit/EffectSpec/PCEffectSpec.h"

#include "GameFramework/WorldSubsystem/PCUnitGERegistrySubsystem.h"


TSubclassOf<UGameplayEffect> UPCEffectSpec::ResolveGEClass(const UWorld* World)
{
	if (CachedGEClass)
		return CachedGEClass;

	if (!World || !EffectKeyTag.IsValid())
		return nullptr;
	
	if (auto* GERegistrySubsystem = World->GetSubsystem<UPCUnitGERegistrySubsystem>())
	{
		if (TSubclassOf<UGameplayEffect> GEClass = GERegistrySubsystem->GetGEClass(EffectKeyTag))
		{
			CachedGEClass = GEClass;
			return GEClass;
		}
	}

	return nullptr;
}
