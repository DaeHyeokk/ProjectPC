// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_CheckTargetInRange.generated.h"

class APCBaseUnitCharacter;
/**
 * 
 */
UCLASS()
class PROJECTPC_API UBTTask_CheckTargetInRange : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_CheckTargetInRange();
	
protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
	UPROPERTY(EditAnywhere, Category="Blackboard")
	FBlackboardKeySelector TargetUnitKey;
	
	void SetTargetActorKey(APCBaseUnitCharacter* Target, UBlackboardComponent* BB) const;

	void ClearTargetActorKey(UBlackboardComponent* BB) const;
};
