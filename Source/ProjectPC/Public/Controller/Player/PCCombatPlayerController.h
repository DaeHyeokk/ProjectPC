// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PCCombatPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTPC_API APCCombatPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	APCCombatPlayerController();
	
protected:
	virtual void SetupInputComponent() override;
	
private:
	// Player의 모든 Input에 대한 MappingContext, Action, Effect가 담긴 DataAsset
	UPROPERTY(EditDefaultsOnly, Category = "DataAsset", meta = (AllowPrivateAccess = "true"))
	class UPCDataAsset_PlayerInput* PlayerInputData;

#pragma region Move
	
private:
	FVector CachedDestination;
	float FollowTime;
	
	void OnInputStarted();
	void OnSetDestinationTriggered();
	void OnSetDestinationReleased();

#pragma endregion Move
};
