// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "PCLobbyGameMode.generated.h"

class APCLobbyGameState;
class APCPlayerState;
/**
 * 
 */
UCLASS()
class PROJECTPC_API APCLobbyGameMode : public AGameMode
{
	GENERATED_BODY()
	
public:
	APCLobbyGameMode() { bUseSeamlessTravel = true;}

	UPROPERTY(EditDefaultsOnly)
	FName GameMap;

	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

	void HandlePlayerEnteredLobby(AController* Controller);

	UFUNCTION()
	void TryAutoStart();
	UFUNCTION()
	void ForceStartByLeader();

	bool AssignSeat(APCPlayerState* PlayerState, int32 NewSeat);

private:
	bool AreAllReady() const;
	void DoTravel();

	APCPlayerState* FindOccupant(int32 SeatIndex) const;
	bool IsSeatFree(int32 SeatIndex, const APCPlayerState* Ignored = nullptr) const;
	
	FTimerHandle CheckTimer;

	bool AssignLeaderIfNone(APCLobbyGameState* LobbyGameState, APCPlayerState* Candidate);
	APCPlayerState* FindCurrentLeader(const APCLobbyGameState* LobbyGameState) const;
	
	
	
};
