// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/UnitCharacter/PCBaseUnitCharacter.h"
#include "PCCreepUnitCharacter.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTPC_API APCCreepUnitCharacter : public APCBaseUnitCharacter
{
	GENERATED_BODY()
	
public:
	virtual UPCUnitAbilitySystemComponent* GetUnitAbilitySystemComponent() const override;
	virtual FGameplayTag GetUnitTypeTag() const override;
	
	
};
