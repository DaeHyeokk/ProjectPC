// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Task/BTTask_FindReleaseLocationNearTarget.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "Character/Unit/PCBaseUnitCharacter.h"
#include "Controller/Unit/PCUnitAIController.h"
#include "Utility/PCUnitCombatUtils.h"


UBTTask_FindReleaseLocationNearTarget::UBTTask_FindReleaseLocationNearTarget()
{
	NodeName = TEXT("Find Release Tile Location Near Target");
}

EBTNodeResult::Type UBTTask_FindReleaseLocationNearTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp,
                                                                       uint8* NodeMemory)
{
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB)
		return EBTNodeResult::Failed;
	
	APCUnitAIController* UnitAIC = Cast<APCUnitAIController>(OwnerComp.GetAIOwner());
	APCBaseUnitCharacter* OwnerUnit = UnitAIC ? Cast<APCBaseUnitCharacter>(UnitAIC->GetPawn()) : nullptr;
	APCBaseUnitCharacter* TargetUnit = Cast<APCBaseUnitCharacter>(BB->GetValueAsObject(TargetUnitKey.SelectedKeyName));
	if (!OwnerUnit || !TargetUnit)
	{
		BB->ClearValue(TargetUnitKey.SelectedKeyName);
		BB->ClearValue(FindLocationKey.SelectedKeyName);
		return EBTNodeResult::Failed;
	}
	
	APCCombatBoard* Board = OwnerUnit->GetOnCombatBoard();
	if (!Board)
	{
		BB->ClearValue(TargetUnitKey.SelectedKeyName);
		BB->ClearValue(FindLocationKey.SelectedKeyName);
		return EBTNodeResult::Failed;
	}
	
	const FIntPoint StartPoint = Board->GetFieldUnitPoint(TargetUnit);
	if (StartPoint == FIntPoint::NoneValue)
	{
		BB->ClearValue(TargetUnitKey.SelectedKeyName);
		BB->ClearValue(FindLocationKey.SelectedKeyName);
		return EBTNodeResult::Failed;
	}
	
	// BFS
	TQueue<FIntPoint> Q;
	TSet<FIntPoint> Visited;
	Q.Enqueue(StartPoint);
	Visited.Add(StartPoint);

	while (!Q.IsEmpty())
	{
		FIntPoint HerePoint;
		Q.Dequeue(HerePoint);
		
		// 탐색 방향 랜덤으로 섞인 Direction 배열 가져옴 (랜덤성 부여)
		for (const FIntPoint& Dir : PCUnitCombatUtils::GetRandomDirections(HerePoint.Y % 2 == 0))
		{
			const FIntPoint NextPoint = HerePoint + Dir;
			if (Board->IsInRange(NextPoint.Y, NextPoint.X) && !Visited.Contains(NextPoint))
			{
				// 탐색한 좌표가 유효한 좌표이고 비어있는 좌표라면 해당 지점을 목표 지점으로 정하고 Task 종료
				if (Board->SetTileState(NextPoint.Y, NextPoint.X, OwnerUnit, ETileAction::Reserve))
				{
					const FIntPoint OwnerPoint = Board->GetFieldUnitPoint(OwnerUnit);
					Board->SetTileState(OwnerPoint.Y, OwnerPoint.X, OwnerUnit, ETileAction::Release);
				
					const FVector NextLocation = Board->GetTileWorldLocation(NextPoint.Y, NextPoint.X);
				
					BB->SetValueAsVector(FindLocationKey.SelectedKeyName, NextLocation);
					UnitAIC->SetCachedPoint(NextPoint, OwnerPoint);
					return EBTNodeResult::Succeeded;
				}

				// 비어있지 않다면 탐색 목록에 추가
				Q.Enqueue(NextPoint);
				Visited.Add(NextPoint);
			}
		}
	}

	// 이곳에 도달했다면 비어있는 좌표 없음 (게임 특성 상 비어있는 좌표는 반드시 존재하기 때문에 이곳에 도달할 가능성은 거의 0%)
	BB->ClearValue(FindLocationKey.SelectedKeyName);
	return EBTNodeResult::Failed;
}
