// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_FindTarget.generated.h"

class APCBaseUnitCharacter;

UENUM(BlueprintType)
enum class ETargetSearchMode : uint8
{
	// 사거리 내 가장 가까이 있는 적
	NearestInRange,
	// 사거리 내 가장 멀리 있는 적
	FarthestInRange,
	// 사거리 상관 없이 가장 멀리 있는 적
	Farthest
	// 추후에 서치 대상 추가가능 ex) LowestHP, HighestDamage
};

/**
 * 
 */
UCLASS()
class PROJECTPC_API UBTTask_FindTarget : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_FindTarget();
	
protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
	UPROPERTY(EditAnywhere, Category="Blackboard")
	FBlackboardKeySelector TargetUnitKey;

	UPROPERTY(EditAnywhere, Category="Data")
	ETargetSearchMode TargetSearchMode = ETargetSearchMode::NearestInRange;
	
private:
	void SetTargetActorKey(APCBaseUnitCharacter* Target, UBlackboardComponent* BB) const;
	void ClearTargetActorKey(UBlackboardComponent* BB) const;
	bool IsTargetInRange(const int8 Range, const int8 TargetDist) const;
};
