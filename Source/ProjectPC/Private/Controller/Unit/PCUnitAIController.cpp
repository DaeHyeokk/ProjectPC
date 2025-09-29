// Fill out your copyright notice in the Description page of Project Settings.


#include "Controller/Unit/PCUnitAIController.h"

#include "BrainComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/Unit/PCBaseUnitCharacter.h"
#include "GameFramework/GameState/PCCombatGameState.h"
#include "Navigation/PathFollowingComponent.h"


APCUnitAIController::APCUnitAIController()
{
	// bWantsPlayerState = true;
	// OwnerUnit = nullptr;
	// CachedMovePoint = FIntPoint::NoneValue;
}

void APCUnitAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (APCBaseUnitCharacter* Unit = Cast<APCBaseUnitCharacter>(InPawn))
	{
		OwnerUnit = Unit;
		const FGenericTeamId PawnTeam = Unit->GetGenericTeamId();
		SetGenericTeamId(PawnTeam);
	}
	
	if (DefaultBT)
	{
		RunBehaviorTree(DefaultBT);
		
		if (UBlackboardComponent* BB = GetBlackboardComponent())
		{
			if (GetWorld())
			{
				APCCombatGameState* CombatGS = GetWorld()->GetGameState<APCCombatGameState>();
				BB->SetValueAsObject(TEXT("CombatGameState"), CombatGS);
			}
		}
	}
}

void APCUnitAIController::OnUnPossess()
{
	Super::OnUnPossess();
	if (UBrainComponent* Brain = GetBrainComponent())
	{
		Brain->StopLogic(TEXT("UnPossess"));
	}
}

void APCUnitAIController::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
	Super::OnMoveCompleted(RequestID, Result);
	
	if (Result.Code == EPathFollowingResult::Success)
	{
		if (OwnerUnit)
		{
			if (const APCCombatBoard* OwnerBoard = OwnerUnit->GetOnCombatBoard())
			{
				const FIntPoint LastPoint = OwnerBoard->GetFieldUnitPoint(OwnerUnit);
				OwnerUnit->GetOnCombatBoard()->SetTileState(LastPoint.Y, LastPoint.X, OwnerUnit, ETileAction::Release);
				OwnerUnit->GetOnCombatBoard()->SetTileState(CachedMovePoint.Y, CachedMovePoint.X, OwnerUnit, ETileAction::Occupy);
			}
		}
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

void APCUnitAIController::ClearBlackboardValue()
{
	if (UBlackboardComponent* BB = GetBlackboardComponent())
	{
		if (GetWorld())
		{
			APCCombatGameState* CombatGS = GetWorld()->GetGameState<APCCombatGameState>();
			BB->ClearValue(TEXT("TargetUnit"));
			BB->ClearValue(TEXT("ApproachLocation"));
		}
	}
}
