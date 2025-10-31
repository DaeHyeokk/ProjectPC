// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_FindReleaseLocationNearTarget.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTPC_API UBTTask_FindReleaseLocationNearTarget : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTask_FindReleaseLocationNearTarget();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	UPROPERTY(EditAnywhere, Category="Blackboard")
	FBlackboardKeySelector TargetUnitKey;

	UPROPERTY(EditAnywhere, Category="Blackboard")
	FBlackboardKeySelector FindLocationKey;
	
	
};
