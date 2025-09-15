// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Task/BTTask_FindApproachLocation.h"

#include "AIController.h"
#include "Algo/RandomShuffle.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/Unit/PCBaseUnitCharacter.h"
#include "Controller/Unit/PCUnitAIController.h"
#include "GameFramework/HelpActor/Component/PCTileManager.h"
#include "Utility/PCGridUtils.h"


UBTTask_FindApproachLocation::UBTTask_FindApproachLocation()
{
	NodeName = TEXT("Find Approach Location To Near Enemy");
}

static bool IsHostile2(const AActor* A, const AActor* B)
{
	const FGenericTeamId TA = FGenericTeamId::GetTeamIdentifier(A);
	const FGenericTeamId TB = FGenericTeamId::GetTeamIdentifier(B);
	return FGenericTeamId::GetAttitude(TA, TB) == ETeamAttitude::Hostile;
}

EBTNodeResult::Type UBTTask_FindApproachLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB)
		return EBTNodeResult::Failed;
	
	APCUnitAIController* UnitAIC = Cast<APCUnitAIController>(OwnerComp.GetAIOwner());
	APCBaseUnitCharacter* OwnerUnit = UnitAIC ? Cast<APCBaseUnitCharacter>(UnitAIC->GetPawn()) : nullptr;
	
	if (!OwnerUnit)
		return EBTNodeResult::Failed;

	APCCombatBoard* Board = OwnerUnit->GetOnCombatBoard();
	if (!Board)
		return EBTNodeResult::Failed;

	const FIntPoint StartPoint = Board->GetFieldUnitPoint(OwnerUnit);
	if (StartPoint == FIntPoint::NoneValue)
		return EBTNodeResult::Failed;

	struct FBfsData
	{
		FIntPoint GridPoint;
		FIntPoint FirstMovePosition;
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

		// 이동할 좌표가 유효한 좌표이고 이동 가능한 좌표라면 이동 방향에 추가
		
		Board->TileManager->DebugExplainTile(NextPoint.Y, NextPoint.X, TEXT("BFS_Neighbor"));
		if (Board->IsInRange(NextPoint.Y, NextPoint.X))
		{
			if (Board->IsTileFree(NextPoint.Y, NextPoint.X))
			{
				Q.Enqueue(FBfsData(NextPoint, NextPoint));
				Visited.Add(NextPoint);
			}
		}
		
	}

	while (!Q.IsEmpty())
	{
		FBfsData HereData;
		Q.Dequeue(HereData);

		const FIntPoint HerePoint = HereData.GridPoint;
		
		Algo::RandomShuffle(ShuffledDirs);
		for (const FIntPoint& Dir : ShuffledDirs)
		{
			const FIntPoint NextPoint = HerePoint + Dir;
			if (Board->IsInRange(NextPoint.Y, NextPoint.X) && !Visited.Contains(NextPoint))
			{
				const APCBaseUnitCharacter* NextUnit = Board->GetUnitAt(NextPoint.Y, NextPoint.X);
				
				// 다음에 탐색할 지점에 유닛이 있고, 적 유닛일 경우
				if (NextUnit && IsHostile2(OwnerUnit, NextUnit))
				{
					const FIntPoint MovePoint = HereData.FirstMovePosition;
					const FVector MoveLocation = Board->GetTileWorldLocation(MovePoint.Y, MovePoint.X);
					BB->SetValueAsVector(ApproachLocationKey.SelectedKeyName, MoveLocation);
					Board->SetTileState(MovePoint.Y, MovePoint.X, OwnerUnit, ETileAction::Reserve);
					UnitAIC->SetMovePoint(MovePoint);
					return EBTNodeResult::Succeeded;
				}

				// 다음에 탐색할 지점이 비어있을 경우
				if (!NextUnit)
				{
					Q.Enqueue(FBfsData(NextPoint, HereData.FirstMovePosition));
					Visited.Add(NextPoint);
				}
			}
		}
	}

	// 이곳에 도달했다면 이동 불가능
	BB->ClearValue(ApproachLocationKey.SelectedKeyName);
	return EBTNodeResult::Failed;
}
