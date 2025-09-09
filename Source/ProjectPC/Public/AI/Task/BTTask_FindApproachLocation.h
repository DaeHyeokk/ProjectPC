// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_FindApproachLocation.generated.h"

class APCBaseUnitCharacter;
/**
 * 
 */
UCLASS()
class PROJECTPC_API UBTTask_FindApproachLocation : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTask_FindApproachLocation();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	UPROPERTY(EditAnywhere, Category="Blackboard")
	FBlackboardKeySelector ApproachLocationKey;
};
