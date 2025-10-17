// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Unit/EffectSpec/PCEffectSpec_SetByCaller.h"

#include "GameFramework/WorldSubsystem/PCUnitGERegistrySubsystem.h"


TSubclassOf<UGameplayEffect> UPCEffectSpec_SetByCaller::ResolveGEClass(const UWorld* World)
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
