// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "PCPlayerRoundRewardExec.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCPlayerRoundRewardExec : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()
	
public:
	UPCPlayerRoundRewardExec();

protected:
	FGameplayEffectAttributeCaptureDefinition PlayerGold;
	
	virtual void Execute_Implementation(
		const FGameplayEffectCustomExecutionParameters& ExecutionParams,
		FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
	
};
