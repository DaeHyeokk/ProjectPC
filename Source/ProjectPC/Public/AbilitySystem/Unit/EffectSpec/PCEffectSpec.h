// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/NoExportTypes.h"
#include "PCEffectSpec.generated.h"

/**
 * 
 */

class UGameplayEffect;
class UAbilitySystemComponent;

UENUM(BlueprintType)
enum class EEffectTargetGroup : uint8
{
	Hostile,
	Ally,
	Self,
	All
};

UCLASS(Abstract, EditInlineNew, DefaultToInstanced, BlueprintType)
class PROJECTPC_API UPCEffectSpec : public UObject
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, Category="Effect")
	EEffectTargetGroup TargetGroup = EEffectTargetGroup::Hostile;

	UPROPERTY(EditDefaultsOnly, Category="Effect")
	FGameplayTag EffectKeyTag;

	UPROPERTY(EditDefaultsOnly, Category="Effect")
	FGameplayTag EffectCallerTag;
	
	virtual void ApplyEffect(UAbilitySystemComponent* SourceASC, AActor* Context) PURE_VIRTUAL(UPCEffectSpec::ApplyEffect, );

protected:
	UPROPERTY(Transient)
	TSubclassOf<UGameplayEffect> CachedGEClass;

	TSubclassOf<UGameplayEffect> ResolveGEClass(const UWorld* World);
};
