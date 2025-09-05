// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_FindTargetInRange.generated.h"

class APCBaseUnitCharacter;

UENUM(BlueprintType)
enum class ETargetSearchMode : uint8
{
	// 사거리 내 가장 가까이 있는 적
	Nearest,
	// 사거리 내 가장 멀리 있는 적
	Farthest,
	
	// 추후에 서치 대상 추가가능 ex) LowestHP, HighestDamage
};

/**
 * 
 */
UCLASS()
class PROJECTPC_API UBTTask_FindTargetInRange : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_FindTargetInRange();
	
protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
	UPROPERTY(EditAnywhere, Category="Blackboard")
	FBlackboardKeySelector TargetActorKey;

	UPROPERTY(EditAnywhere, Category="Data")
	ETargetSearchMode TargetSearchMode = ETargetSearchMode::Nearest;

private:
	void SetTargetActorKey(APCBaseUnitCharacter* Target, UBlackboardComponent* BB) const;
};
