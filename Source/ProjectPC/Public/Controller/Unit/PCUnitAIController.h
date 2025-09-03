// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "PCUnitAIController.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTPC_API APCUnitAIController : public AAIController
{
	GENERATED_BODY()
	
protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	
public:
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UBehaviorTree> DefaultBT;
	
};
