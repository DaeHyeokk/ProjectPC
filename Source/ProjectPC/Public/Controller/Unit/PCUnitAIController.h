// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "GameplayTagContainer.h"
#include "PCUnitAIController.generated.h"

class APCBaseUnitCharacter;
class APCCombatGameState;

/**
 * 
 */
UCLASS()
class PROJECTPC_API APCUnitAIController : public AAIController
{
	GENERATED_BODY()

protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	virtual void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result) override;

private:
	UPROPERTY(Transient)
	FIntPoint CachedMovePoint;

	bool bIsMoving = false;
	
public:
	void UpdateTeamId();
	void SetMovePoint(const FIntPoint& MovePoint);
	void ClearBlackboardValue();
	
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UBehaviorTree> DefaultBT;

private:
	UFUNCTION()
	void HandleGameStateTagChanged(const FGameplayTag& ChangedTag);

	UFUNCTION()
	void HandleUnitStateTagChanged(FGameplayTag ChangedTag, int32 NewCount);

	void BindUnitASCDelegates();
	void UnBindUnitASCDelegates();

	
	UPROPERTY()
	TWeakObjectPtr<APCBaseUnitCharacter> OwnerUnit;

	TWeakObjectPtr<APCCombatGameState> CachedCombatGS;

	FDelegateHandle OnGameStateTagChangedHandle;
	FDelegateHandle OnStunTagHandle;
	FDelegateHandle OnDeadTagHandle;
};
