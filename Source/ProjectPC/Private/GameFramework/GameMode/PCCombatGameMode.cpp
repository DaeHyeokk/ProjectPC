// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFramework/GameMode/PCCombatGameMode.h"

#include "GameFramework/GameState.h"
#include "GameFramework/PlayerState/PCPlayerState.h"


void APCCombatGameMode::PostSeamlessTravel()
{
	Super::PostSeamlessTravel();
	if (auto* GS = GetGameState<AGameState>())
	{
		for (APlayerState* PS : GS->PlayerArray)
			if (auto* PCPlayerState = Cast<APCPlayerState>(PS))
				PCPlayerState->bIsReady = false;
	}
}
