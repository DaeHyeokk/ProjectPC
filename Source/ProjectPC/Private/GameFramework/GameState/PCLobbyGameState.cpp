// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFramework/GameState/PCLobbyGameState.h"

#include "Controller/Player/PCLobbyPlayerController.h"
#include "GameFramework/PlayerState/PCPlayerState.h"
#include "Net/UnrealNetwork.h"



void APCLobbyGameState::RecountReady()
{
	if (!HasAuthority()) return;
	
	int32 ReadyCount = 0;
	int32 PlayersCount = 0;

	for (APlayerState* PS : PlayerArray)
	{
		if (const APCPlayerState* PCPlayerState = Cast<APCPlayerState>(PS))
		{
			if (!PCPlayerState->bIdentified) continue;
			++PlayersCount;
			if (PCPlayerState->bIsReady)
			{
				++ReadyCount;
			}
		}
	}
	NumPlayersReady = ReadyCount;
	NumPlayers = PlayersCount;
}

void APCLobbyGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(APCLobbyGameState, NumPlayersReady);
	DOREPLIFETIME(APCLobbyGameState, NumPlayers);
	DOREPLIFETIME(APCLobbyGameState, bCountDown);
	DOREPLIFETIME(APCLobbyGameState, CountdownEndServerTime);
}
