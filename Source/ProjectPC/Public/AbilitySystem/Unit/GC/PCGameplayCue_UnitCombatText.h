// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseGameplayTags.h"
#include "GameplayCueNotify_Static.h"
#include "PCGameplayCue_UnitCombatText.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCGameplayCue_UnitCombatText : public UGameplayCueNotify_Static
{
	GENERATED_BODY()
	
public:
	virtual bool OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const override;
	
};
