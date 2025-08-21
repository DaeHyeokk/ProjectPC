// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFramework/PlayerState/PCPlayerState.h"

#include "Net/UnrealNetwork.h"


void APCPlayerState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(APCPlayerState, bIsReady);
	DOREPLIFETIME(APCPlayerState, LocalUserId);
	DOREPLIFETIME(APCPlayerState, bIsLeader);
	DOREPLIFETIME(APCPlayerState, SeatIndex);
	DOREPLIFETIME(APCPlayerState, bIdentified);
}
