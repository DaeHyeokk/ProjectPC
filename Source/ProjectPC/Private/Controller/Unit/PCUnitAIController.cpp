// Fill out your copyright notice in the Description page of Project Settings.


#include "Controller/Unit/PCUnitAIController.h"

#include "AbilitySystemComponent.h"
#include "BrainComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/Unit/PCBaseUnitCharacter.h"
#include "GameFramework/GameState/PCCombatGameState.h"
#include "Navigation/PathFollowingComponent.h"


void APCUnitAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (APCBaseUnitCharacter* Unit = Cast<APCBaseUnitCharacter>(InPawn))
	{
		OwnerUnit = Unit;
		const FGenericTeamId PawnTeam = Unit->GetGenericTeamId();
		SetGenericTeamId(PawnTeam);
		BindUnitASCDelegates();
	}
	
	if (DefaultBT)
	{
		RunBehaviorTree(DefaultBT);
		
		if (UBlackboardComponent* BB = GetBlackboardComponent())
		{
			if (UWorld* World = GetWorld())
			{
				if (APCCombatGameState* CombatGS = World->GetGameState<APCCombatGameState>())
				{
					BB->SetValueAsObject(TEXT("CombatGameState"), CombatGS);
					CachedCombatGS = CombatGS;
					
					if (OnGameStateTagChangedHandle.IsValid())
					{
						CombatGS->OnGameStateTagChanged.Remove(OnGameStateTagChangedHandle);
						OnGameStateTagChangedHandle.Reset();
					}

					OnGameStateTagChangedHandle = CombatGS->OnGameStateTagChanged.AddUObject(
						this, &ThisClass::HandleGameStateTagChanged);
				}
			}
		}
	}
}

void APCUnitAIController::OnUnPossess()
{
	UnBindUnitASCDelegates();
	
	if (CachedCombatGS.IsValid())
	{
		CachedCombatGS->OnGameStateTagChanged.Remove(OnGameStateTagChangedHandle);
		OnGameStateTagChangedHandle.Reset();
	}
	
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
		if (OwnerUnit.IsValid())
		{
			if (const APCCombatBoard* OwnerBoard = OwnerUnit->GetOnCombatBoard())
			{
				const FIntPoint LastPoint = OwnerBoard->GetFieldUnitPoint(OwnerUnit.Get());
				OwnerUnit->GetOnCombatBoard()->SetTileState(LastPoint.Y, LastPoint.X, OwnerUnit.Get(), ETileAction::Release);
				OwnerUnit->GetOnCombatBoard()->SetTileState(CachedMovePoint.Y, CachedMovePoint.X, OwnerUnit.Get(), ETileAction::Occupy);
				bIsMoving = false;
			}
			
			GetBlackboardComponent()->ClearValue(TEXT("ApproachLocation"));
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

void APCUnitAIController::SetMovePoint(const FIntPoint& MovePoint)
{
	CachedMovePoint = MovePoint;
	bIsMoving = true;
}

void APCUnitAIController::ClearBlackboardValue()
{
	if (UBlackboardComponent* BB = GetBlackboardComponent())
	{
		BB->ClearValue(TEXT("TargetUnit"));
		BB->ClearValue(TEXT("ApproachLocation"));
	}
}

void APCUnitAIController::HandleGameStateTagChanged(const FGameplayTag& ChangedTag)
{
	if (!CachedCombatGS.IsValid())
		return;

	if (UBlackboardComponent* BB = GetBlackboardComponent())
	{
		const bool bCombatActive = ChangedTag.MatchesTag(GameStateTags::Game_State_Combat_Active);
		BB->SetValueAsBool(TEXT("IsCombatActive"), bCombatActive);
	}
}

void APCUnitAIController::HandleUnitStateTagChanged(FGameplayTag ChangedTag, int32 NewCount)
{
	const bool bActive = (NewCount > 0);

	if (UBlackboardComponent* BB = GetBlackboardComponent())
	{
		if (ChangedTag.MatchesTagExact(UnitGameplayTags::Unit_State_Combat_Dead))
		{
			BB->SetValueAsBool(TEXT("IsDead"), bActive);
		}
		else if (ChangedTag.MatchesTagExact(UnitGameplayTags::Unit_State_Combat_Stun))
		{
			BB->SetValueAsBool(TEXT("IsStun"), bActive);
		}
	}
}

void APCUnitAIController::BindUnitASCDelegates()
{
	if (!OwnerUnit.IsValid())
		return;

	if (UAbilitySystemComponent* ASC = OwnerUnit->GetAbilitySystemComponent())
	{
		OnDeadTagHandle = ASC->RegisterGameplayTagEvent(UnitGameplayTags::Unit_State_Combat_Dead)
		.AddUObject(this, &ThisClass::HandleUnitStateTagChanged);

		OnStunTagHandle = ASC->RegisterGameplayTagEvent(UnitGameplayTags::Unit_State_Combat_Stun)
		.AddUObject(this, &ThisClass::HandleUnitStateTagChanged);

		HandleUnitStateTagChanged(UnitGameplayTags::Unit_State_Combat_Dead,
			ASC->GetGameplayTagCount(UnitGameplayTags::Unit_State_Combat_Dead));
		HandleUnitStateTagChanged(UnitGameplayTags::Unit_State_Combat_Stun,
			ASC->GetGameplayTagCount(UnitGameplayTags::Unit_State_Combat_Stun));
	}
}

void APCUnitAIController::UnBindUnitASCDelegates()
{
	if (!OwnerUnit.IsValid())
		return;
	
	if (UAbilitySystemComponent* ASC = OwnerUnit->GetAbilitySystemComponent())
	{
		if (OnDeadTagHandle.IsValid())
			ASC->RegisterGameplayTagEvent(UnitGameplayTags::Unit_State_Combat_Dead).Remove(OnDeadTagHandle);

		if (OnStunTagHandle.IsValid())
			ASC->RegisterGameplayTagEvent(UnitGameplayTags::Unit_State_Combat_Stun).Remove(OnStunTagHandle);
	}
}
