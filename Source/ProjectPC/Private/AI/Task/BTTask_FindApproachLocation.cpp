// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Task/BTTask_FindApproachLocation.h"

#include "AIController.h"
#include "Algo/RandomShuffle.h"
#include "Character/UnitCharacter/PCBaseUnitCharacter.h"
#include "Utility/PCGridUtils.h"


UBTTask_FindApproachLocation::UBTTask_FindApproachLocation()
{
	NodeName = TEXT("Find Approach Location To Near Enemy");
}

EBTNodeResult::Type UBTTask_FindApproachLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB)
		return EBTNodeResult::Failed;
	
	AAIController* AIC = OwnerComp.GetAIOwner();
	APCBaseUnitCharacter* OwnerUnit = AIC ? Cast<APCBaseUnitCharacter>(AIC->GetPawn()) : nullptr;
	
	if (!OwnerUnit)
		return EBTNodeResult::Failed;

	const APCCombatBoard* Board = OwnerUnit->GetOnCombatBoard();
	if (!Board)
		return EBTNodeResult::Failed;

	const FIntPoint StartPoint = Board->GetFiledUnitPoint(OwnerUnit);
	if (StartPoint == FIntPoint::NoneValue)
		return EBTNodeResult::Failed;

	struct FBfsData
	{
		FIntPoint GridPoint;
		FIntPoint FirstMoveDirection;
	};
	
	// BFS
	TQueue<FBfsData> Q;
	TSet<FIntPoint> Visited;
	Visited.Add(StartPoint);

	// 탐색 방향 랜덤으로 섞음 (랜덤성 부여)
	TArray<FIntPoint> ShuffledDirs;
	ShuffledDirs.Append(PCGridUtils::Directions, UE_ARRAY_COUNT(PCGridUtils::Directions));
	Algo::RandomShuffle(ShuffledDirs);

	for (const FIntPoint& Dir : ShuffledDirs)
	{
		FIntPoint NextPoint = StartPoint + Dir;
		
	}

	return EBTNodeResult::Failed;
}
