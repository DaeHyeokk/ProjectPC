// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Unit/EffectSpec/PCEffectSpec.h"
#include "PCEffectSpec_SetByCaller.generated.h"

/**
 * 
 */
UCLASS(Abstract)
class PROJECTPC_API UPCEffectSpec_SetByCaller : public UPCEffectSpec
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category="Effect", meta=(Categories="GE.Class"))
	FGameplayTag EffectClassTag;
	
	UPROPERTY(EditDefaultsOnly, Category="Effect", meta=(Categories="GE.Caller"))
	FGameplayTag EffectCallerTag;

protected:
	UPROPERTY(Transient)
	TSubclassOf<UGameplayEffect> CachedGEClass;

	TSubclassOf<UGameplayEffect> ResolveGEClass(const UWorld* World);
};
