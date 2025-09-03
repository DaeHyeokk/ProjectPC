// Fill out your copyright notice in the Description page of Project Settings.


#include "Controller/Unit/PCUnitAIController.h"

#include "BrainComponent.h"


void APCUnitAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (DefaultBT)
		RunBehaviorTree(DefaultBT);
}

void APCUnitAIController::OnUnPossess()
{
	Super::OnUnPossess();
	if (UBrainComponent* Brain = GetBrainComponent())
	{
		Brain->StopLogic(TEXT("UnPossess"));
	}
}
