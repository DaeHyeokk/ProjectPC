// Fill out your copyright notice in the Description page of Project Settings.


#include "Controller/Unit/PCUnitAIController.h"

#include "BrainComponent.h"
#include "Character/UnitCharacter/PCBaseUnitCharacter.h"


void APCUnitAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (const APCBaseUnitCharacter* Unit = Cast<APCBaseUnitCharacter>(InPawn))
	{
		const FGenericTeamId PawnTeam = Unit->GetGenericTeamId();
		SetGenericTeamId(PawnTeam);
	}
	
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

void APCUnitAIController::UpdateTeamId()
{
	if (const APCBaseUnitCharacter* Unit = Cast<APCBaseUnitCharacter>(GetOwner()))
	{
		const FGenericTeamId PawnTeam = Unit->GetGenericTeamId();
		SetGenericTeamId(PawnTeam);
	}
}