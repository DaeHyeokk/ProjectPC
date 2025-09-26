// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFramework/GameMode/PCLobbyGameMode.h"

#include "NetworkMessage.h"
#include "Engine/LevelScriptBlueprint.h"
#include "GameFramework/GameState/PCLobbyGameState.h"
#include "GameFramework/PlayerState/PCPlayerState.h"


void APCLobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (!GetWorldTimerManager().IsTimerActive(CheckTimer))
		GetWorldTimerManager().SetTimer(CheckTimer, this, &APCLobbyGameMode::TryAutoStart, 1.0f, true);

	if (auto* LobbyGameState = GetGameState<APCLobbyGameState>())
		LobbyGameState->RecountReady();
}

void APCLobbyGameMode::Logout(AController* Exiting)
{
	APCPlayerState* PS = Exiting ? Exiting->GetPlayerState<APCPlayerState>() : nullptr;
	const bool bWasLeader = PS ? PS->bIsLeader : false;

	if (PS) PS->SeatIndex = -1;
	Super::Logout(Exiting);

	if (APCLobbyGameState* GS = GetGameState<APCLobbyGameState>())
	{
		// 리더가 나갔으면 새 리더 선출 (좌석 낮은 사람/첫 사람 등 규칙 자유)
		if (bWasLeader)
		{
			APCPlayerState* NewLeader = nullptr;
			for (APlayerState* S : GS->PlayerArray)
				if (APCPlayerState* P = Cast<APCPlayerState>(S))
				{ NewLeader = P; break; } // 단순히 첫 번째로
			if (NewLeader) NewLeader->bIsLeader = true;
		}
		GS->RecountReady();
	}
}

void APCLobbyGameMode::HandlePlayerEnteredLobby(AController* Controller)
{
	auto* LobbyGameState = GetGameState<APCLobbyGameState>();
	auto* PlayerState = Controller ? Controller->GetPlayerState<APCPlayerState>() : nullptr;
	if (!GameState || !PlayerState) return;

	if (PlayerState->SeatIndex >= 0)
		return;

	PlayerState->SetPlayerName(TEXT(""));

	AssignLeaderIfNone(LobbyGameState, PlayerState);

	if (PlayerState->SeatIndex < 0)
	{
		for (int32 i = 0; i < LobbyGameState->NumSeats; ++i)
			if (IsSeatFree(i)) { AssignSeat(PlayerState, i); break; }
	}

	LobbyGameState->RecountReady();
}

void APCLobbyGameMode::TryAutoStart()
{
	if (AreAllReady())
		DoTravel();
}

void APCLobbyGameMode::ForceStartByLeader()
{
	DoTravel();
}

bool APCLobbyGameMode::AssignSeat(APCPlayerState* PlayerState, int32 NewSeat)
{
	APCLobbyGameState* LobbyGameState = GetGameState<APCLobbyGameState>();
	if (!LobbyGameState || !PlayerState || NewSeat < 0 || NewSeat >= LobbyGameState->NumSeats)
		return false;

	if (PlayerState->SeatIndex == NewSeat)
		return true;

	APCPlayerState* Other = FindOccupant(NewSeat);
	const int32 OldSeat = PlayerState->SeatIndex;

	if (!Other)
	{
		PlayerState->SeatIndex = NewSeat;
		return true;
	}

	Other->SeatIndex = OldSeat;
	PlayerState->SeatIndex = NewSeat;
	return true;
}

APCPlayerState* APCLobbyGameMode::FindOccupant(int32 SeatIndex) const
{
	const APCLobbyGameState* LobbyGameState = GetGameState<APCLobbyGameState>();
	if (!LobbyGameState) return nullptr;

	for (APlayerState* PlayerState : LobbyGameState->PlayerArray)
		if (auto* PCPlayerState = Cast<APCPlayerState>(PlayerState))
			if (PCPlayerState->SeatIndex == SeatIndex) return PCPlayerState;
	return nullptr;
}

bool APCLobbyGameMode::IsSeatFree(int32 SeatIndex, const APCPlayerState* Ignored) const
{
	return FindOccupant(SeatIndex) == nullptr || FindOccupant(SeatIndex) == Ignored;
}

bool APCLobbyGameMode::AssignLeaderIfNone(APCLobbyGameState* LobbyGameState, APCPlayerState* Candidate)
{
	if (!LobbyGameState || !Candidate) return false;
	if (FindCurrentLeader(LobbyGameState)) return false;
	Candidate->bIsLeader = true;
	return true;
}

APCPlayerState* APCLobbyGameMode::FindCurrentLeader(const APCLobbyGameState* LobbyGameState) const
{
	if (!LobbyGameState) return nullptr;
	for (APlayerState* PlayerState : LobbyGameState->PlayerArray)
	{
		if (const APCPlayerState* PCPlayerState = Cast<APCPlayerState>(PlayerState))
		{
			if (PCPlayerState->bIsLeader)
			{
				return const_cast<APCPlayerState*>(PCPlayerState);
			}
		}
	}
	return nullptr;
}

bool APCLobbyGameMode::AreAllReady() const
{
	if (const auto* GS = GetGameState<APCLobbyGameState>())
		return GS->PlayerArray.Num() > 0 && GS->NumPlayersReady == GS->PlayerArray.Num();
	return false;
}

void APCLobbyGameMode::DoTravel()
{	
	if (!HasAuthority() || LevelToTravel.IsNull()) return;

	const int32 Expected = GetGameState<AGameStateBase>()->PlayerArray.Num(); // or Ready 인원
	const FString LevelName = LevelToTravel.ToSoftObjectPath().GetLongPackageName();
	const FString Url = FString::Printf(TEXT("%s?listen?ExpPlayers=%d"), *LevelName, Expected);
	GetWorld()->ServerTravel(Url);
}
