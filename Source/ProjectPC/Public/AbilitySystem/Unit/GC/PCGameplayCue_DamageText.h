// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseGameplayTags.h"
#include "GameplayCueNotify_Static.h"
#include "PCGameplayCue_DamageText.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCGameplayCue_DamageText : public UGameplayCueNotify_Static
{
	GENERATED_BODY()
	
public:
	virtual bool OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const override;
	
protected:
	const FGameplayTag PhysicalDamageTypeTag = UnitGameplayTags::Unit_DamageType_Physical;
	const FGameplayTag MagicDamageTypeTag = UnitGameplayTags::Unit_DamageType_Magic;
	
};
