// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Task/BTTask_CheckTargetInRange.h"

#include "AbilitySystemComponent.h"
#include "AIController.h"
#include "BaseGameplayTags.h"
#include "AbilitySystem/Unit/AttributeSet/PCUnitAttributeSet.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/Unit/PCBaseUnitCharacter.h"
#include "Utility/PCUnitCombatUtils.h"


UBTTask_CheckTargetInRange::UBTTask_CheckTargetInRange()
{
	NodeName = TEXT("Is Valid Target");
}

EBTNodeResult::Type UBTTask_CheckTargetInRange::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB)
		return EBTNodeResult::Failed;

	const APCBaseUnitCharacter* TargetUnit = Cast<APCBaseUnitCharacter>(BB->GetValueAsObject(TargetUnitKey.SelectedKeyName));
	if (!TargetUnit)
	{
		ClearTargetActorKey(BB);
		return EBTNodeResult::Failed;
	}

	const UAbilitySystemComponent* TargetASC = TargetUnit->GetAbilitySystemComponent();
	// 타겟이 죽었을 경우 타겟 리셋
	if (!TargetASC || TargetASC->HasMatchingGameplayTag(UnitGameplayTags::Unit_State_Combat_Dead))
	{
		ClearTargetActorKey(BB);
		return EBTNodeResult::Failed;
	}
	
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
			if (TargetUnit == HereUnit)
				return EBTNodeResult::Succeeded;
		}

		// 현재까지 탐색한 거리가 Range보다 크거나 같다면 더 멀리 탐색하지 않음
		if (HereDist >= Range)
		{
			continue;
		}
		
		// 탐색 방향 랜덤으로 섞인 Direction 배열 가져옴 (랜덤성 부여) / 현재 좌표 Y값이 홀수인지 짝수인지에 따라 다른 배열 가져옴
		for (const FIntPoint& Dir : PCUnitCombatUtils::GetRandomDirections(HerePoint.Y % 2 == 0))
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
	
	// 설정한 타겟이 사거리 내에 존재하지 않을 경우 false
	ClearTargetActorKey(BB);
	return EBTNodeResult::Failed;
}

void UBTTask_CheckTargetInRange::SetTargetActorKey(APCBaseUnitCharacter* Target, UBlackboardComponent* BB) const
{
	BB->SetValueAsObject(TargetUnitKey.SelectedKeyName, Target);
}

void UBTTask_CheckTargetInRange::ClearTargetActorKey(UBlackboardComponent* BB) const
{
	BB->ClearValue(TargetUnitKey.SelectedKeyName);
}
