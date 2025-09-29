// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "PCUnitAIController.generated.h"

class APCBaseUnitCharacter;
/**
 * 
 */
UCLASS()
class PROJECTPC_API APCUnitAIController : public AAIController
{
	GENERATED_BODY()

protected:
	APCUnitAIController();
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	virtual void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result) override;

private:
	FAIRequestID ActiveMoveId;

	UPROPERTY(Transient)
	FIntPoint CachedMovePoint;
	
public:
	void UpdateTeamId();
	void SetMovePoint(const FIntPoint& MovePoint) { CachedMovePoint = MovePoint; }
	void ClearBlackboardValue();
	
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UBehaviorTree> DefaultBT;

	UPROPERTY(Transient)
	APCBaseUnitCharacter* OwnerUnit;
};
