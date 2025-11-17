// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_TryJumpAbility.generated.h"

class UGameplayAbility;
class UAbilitySystemComponent;
/**
 * 
 */
UCLASS()
class PROJECTPC_API UBTTask_TryJumpAbility : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTask_TryJumpAbility();
	
protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	UPROPERTY(EditAnywhere, Category="Blackboard")
	FBlackboardKeySelector JumpLocationKey;
};
