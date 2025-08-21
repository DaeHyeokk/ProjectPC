// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "PCLobbyGameState.generated.h"

/**
 * 
 */

class APCPlayerState;

UCLASS()
class PROJECTPC_API APCLobbyGameState : public AGameState
{
	GENERATED_BODY()

public:


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lobby")
	int32 NumSeats = 8;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Lobby")
	int32 NumPlayersReady = 0;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Lobby")
	int32 NumPlayers = 0;
	
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Lobby|Countdown")
	bool bCountDown = false;

	UPROPERTY(Replicated,BlueprintReadOnly, Category = "Lobby|Countdown")
	float CountdownEndServerTime = 0.f;
	
	UFUNCTION()
	void RecountReady();
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	
	
};
