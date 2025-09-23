// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Task/BTTask_FindTarget.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/Unit/AttributeSet/PCUnitAttributeSet.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/Unit/PCBaseUnitCharacter.h"
#include "Containers/Queue.h"
#include "Controller/Unit/PCUnitAIController.h"
#include "Utility/PCUnitCombatUtils.h"
#include "Algo/RandomShuffle.h"

UBTTask_FindTarget::UBTTask_FindTarget()
{
	NodeName = TEXT("Find Target In Attack Range");
}

EBTNodeResult::Type UBTTask_FindTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB)
		return EBTNodeResult::Failed;
	
	AAIController* AIC = OwnerComp.GetAIOwner();
	APCBaseUnitCharacter* OwnerUnit = AIC ? Cast<APCBaseUnitCharacter>(AIC->GetPawn()) : nullptr;
	
	if (!OwnerUnit)
	{
		ClearTargetActorKey(BB);
		return EBTNodeResult::Failed;
	}
	
	const APCCombatBoard* Board = OwnerUnit->GetOnCombatBoard();
	if (!Board)
	{
		ClearTargetActorKey(BB);
		return EBTNodeResult::Failed;
	}
	
	const UAbilitySystemComponent* ASC = OwnerUnit->GetAbilitySystemComponent();
	const UPCUnitAttributeSet* AttrSet = ASC ? ASC->GetSet<UPCUnitAttributeSet>() : nullptr;
	if (!AttrSet)
	{
		ClearTargetActorKey(BB);
		return EBTNodeResult::Failed;
	}
	
	const int8 Range = static_cast<int8>(AttrSet->GetAttackRange());
	
	const FIntPoint StartPoint = Board->GetFieldUnitPoint(OwnerUnit);
	if (StartPoint == FIntPoint::NoneValue)
	{
		ClearTargetActorKey(BB);
		return EBTNodeResult::Failed;
	}
	
	struct FBfsData
	{
		FIntPoint GridPoint;
		// 격자 크기가 7X8 = 56, 최대 탐색 거리가 56이므로 255를 넘지 않는다는 보장이 되어있기 때문에 int8 사용
		int8 Dist;
	};
	
	// BFS
	TQueue<FBfsData> Q;
	TSet<FIntPoint> Visited;
	Q.Enqueue(FBfsData(StartPoint, 0));
	Visited.Add(StartPoint);
	
	APCBaseUnitCharacter* Farthest = nullptr;
	
	while (!Q.IsEmpty())
	{
		FBfsData HereData;
		Q.Dequeue(HereData);

		const FIntPoint HerePoint = HereData.GridPoint;
		const int8 HereDist = HereData.Dist;
		APCBaseUnitCharacter* HereUnit = Board->GetUnitAt(HerePoint.Y, HerePoint.X);

		// 현재 유닛이 유효하고, 자기 자신이 아니며, 사거리 내에 있고, 적일 경우
		if (HereUnit && OwnerUnit != HereUnit && Range >= HereDist && PCUnitCombatUtils::IsHostile(OwnerUnit, HereUnit))
		{
			switch (TargetSearchMode)
			{
				// 가장 가까이 있는 적을 찾는거라면 바로 Succeeded 반환
			case ETargetSearchMode::NearestInRange:
				SetTargetActorKey(HereUnit, BB);
				return EBTNodeResult::Succeeded;
				
				// 가장 멀리 있는 적을 찾는거라면 현재 적을 Target 후보에 추가
			case ETargetSearchMode::FarthestInRange:
				Farthest = HereUnit;
				break;

			default:
				break;
			}
		}

		// 현재까지 탐색한 거리가 Range보다 크거나 같다면 더 멀리 탐색하지 않음
		if (TargetSearchMode != ETargetSearchMode::Farthest)
		{
			if (HereDist >= Range)
			{
				continue;
			}
		}
		
		// 탐색 방향 랜덤으로 섞인 Direction 배열 가져옴 (랜덤성 부여)
		for (const FIntPoint& Dir : PCUnitCombatUtils::GetRandomDirections())
		{
			const FIntPoint NextPoint = HerePoint + Dir;
			
			// 다음 탐색 좌표가 보드 내에 존재하는 좌표이고 아직 방문하지 않은 좌표라면 탐색
			if (Board->IsInRange(NextPoint.Y, NextPoint.X) && !Visited.Contains(NextPoint))
			{
				// 새롭게 탐색할 좌표를 큐에 넣고 Visited 방문 체크
				Q.Enqueue(FBfsData(NextPoint, HereDist + 1));
				Visited.Add(NextPoint);
			}
		}
	}

	if (Farthest)
	{
		// 찾은 타겟이 있다면 타겟을 TargetActorKey에 할당하고 Succeeded 반환
		SetTargetActorKey(Farthest, BB);
		return EBTNodeResult::Succeeded;
	}
	else
	{
		// 찾은 타겟이 없다면 TargetActorKey를 클리어 하고 Failed 반환
		ClearTargetActorKey(BB);
		return EBTNodeResult::Failed;
	}
}

void UBTTask_FindTarget::SetTargetActorKey(APCBaseUnitCharacter* Target, UBlackboardComponent* BB) const
{
	BB->SetValueAsObject(TargetUnitKey.SelectedKeyName, Target);
}

void UBTTask_FindTarget::ClearTargetActorKey(UBlackboardComponent* BB) const
{
	BB->ClearValue(TargetUnitKey.SelectedKeyName);
}
