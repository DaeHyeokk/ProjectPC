// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PCEffectCoreTypes.generated.h"

class UPCEffectSpec;

USTRUCT(BlueprintType)
struct FPCEffectSpecList
{
	GENERATED_BODY()
	UPROPERTY(EditDefaultsOnly, Instanced)
	TArray<TObjectPtr<UPCEffectSpec>> EffectSpecs;

	bool IsEmpty() const { return EffectSpecs.IsEmpty(); }
};