// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Task/BTTask_FindApproachLocation.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/Unit/PCBaseUnitCharacter.h"
#include "Controller/Unit/PCUnitAIController.h"
#include "Containers/Queue.h"
#include "Utility/PCUnitCombatUtils.h"
#include "GameFramework/HelpActor/Component/PCTileManager.h"


UBTTask_FindApproachLocation::UBTTask_FindApproachLocation()
{
	NodeName = TEXT("Find Approach Location To Near Enemy");
}

EBTNodeResult::Type UBTTask_FindApproachLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB)
		return EBTNodeResult::Failed;
	
	APCUnitAIController* UnitAIC = Cast<APCUnitAIController>(OwnerComp.GetAIOwner());
	APCBaseUnitCharacter* OwnerUnit = UnitAIC ? Cast<APCBaseUnitCharacter>(UnitAIC->GetPawn()) : nullptr;
	
	if (!OwnerUnit)
	{
		BB->ClearValue(ApproachLocationKey.SelectedKeyName);
		return EBTNodeResult::Failed;
	}
	
	APCCombatBoard* Board = OwnerUnit->GetOnCombatBoard();
	if (!Board)
	{
		BB->ClearValue(ApproachLocationKey.SelectedKeyName);
		return EBTNodeResult::Failed;
	}
	
	const FIntPoint StartPoint = Board->GetFieldUnitPoint(OwnerUnit);
		
	if (StartPoint == FIntPoint::NoneValue)
	{
		BB->ClearValue(ApproachLocationKey.SelectedKeyName);
		return EBTNodeResult::Failed;
	}
	struct FBfsData
	{
		FIntPoint GridPoint;
		FIntPoint FirstMovePosition;
	};
	
	// BFS
	TQueue<FBfsData> Q;
	TSet<FIntPoint> Visited;
	Visited.Add(StartPoint);

	// 탐색 방향 랜덤으로 섞인 Direction 배열 가져옴 (랜덤성 부여)
	for (const FIntPoint& Dir : PCUnitCombatUtils::GetRandomDirections(StartPoint.Y % 2 == 0))
	{
		FIntPoint NextPoint = StartPoint + Dir;
		
		// 이동할 좌표가 유효한 좌표이고 이동 가능한 좌표라면 이동 방향에 추가
		if (Board->IsInRange(NextPoint.Y, NextPoint.X) && Board->IsTileFree(NextPoint.Y, NextPoint.X))
		{
			Q.Enqueue(FBfsData(NextPoint, NextPoint));
			Visited.Add(NextPoint);
		}
	}

	while (!Q.IsEmpty())
	{
		FBfsData HereData;
		Q.Dequeue(HereData);

		const FIntPoint HerePoint = HereData.GridPoint;
		
		for (const FIntPoint& Dir : PCUnitCombatUtils::GetRandomDirections(HerePoint.Y % 2 == 0))
		{
			const FIntPoint NextPoint = HerePoint + Dir;
			
			if (Board->IsInRange(NextPoint.Y, NextPoint.X) && !Visited.Contains(NextPoint))
			{
				
				const APCBaseUnitCharacter* NextUnit = Board->GetUnitAt(NextPoint.Y, NextPoint.X);
				
				// 다음에 탐색할 지점에 유닛이 있고, 적 유닛일 경우
				if (NextUnit && PCUnitCombatUtils::IsHostile(OwnerUnit, NextUnit))
				{
					FIntPoint MovePoint = HereData.FirstMovePosition;
					const FVector MoveLocation = Board->GetTileWorldLocation(MovePoint.Y, MovePoint.X);

					//UE_LOG(LogTemp, Warning, TEXT("UnitName : %s, MovePoint : X = %d, Y = %d"),*OwnerUnit->GetName(), MovePoint.X, MovePoint.Y)
					UE_LOG(LogTemp, Warning, TEXT("Unit Name : %s, MoveLocation : X = %f, Y = %f, Z = %f"),*OwnerUnit->GetName(),MoveLocation.X, MoveLocation.Y, MoveLocation.Z)
					
					if (Board->SetTileState(MovePoint.Y, MovePoint.X, OwnerUnit, ETileAction::Reserve))
					{
						BB->SetValueAsVector(ApproachLocationKey.SelectedKeyName, MoveLocation);
						UnitAIC->SetCachedPoint(MovePoint, StartPoint);
						return EBTNodeResult::Succeeded;
					}
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
