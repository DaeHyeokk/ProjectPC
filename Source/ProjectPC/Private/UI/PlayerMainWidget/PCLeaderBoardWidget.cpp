// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PlayerMainWidget/PCLeaderBoardWidget.h"

#include "Components/VerticalBox.h"
#include "GameFramework/GameState/PCCombatGameState.h"
#include "UI/PlayerMainWidget/PCPlayerRowWidget.h"


void UPCLeaderBoardWidget::BindToGameState(APCCombatGameState* NewGameState)
{
	// if (!NewGameState || !PlayerBox) return;
	//
	// NewGameState->OnLeaderboardMapUpdated.AddUObject(this, &UPCLeaderBoardWidget::SetupLeaderBoard);
	//
	// for (const auto& PlayerRow : NewGameState->Leaderboard)
	// {
	// 	auto PlayerRowWidget = CreateWidget<UPCPlayerRowWidget>(GetWorld(), PlayerRowWidgetClass);
	// 	if (!PlayerRowWidget) continue;
	//
	// 	PlayerRowWidget->SetupPlayerInfo(PlayerRow.LocalUserId, PlayerRow.Hp, PlayerRow.CharacterTag);
	// 	PlayerMap.Add(PlayerRow.LocalUserId, PlayerRowWidget);
	// 	PlayerBox->AddChild(PlayerRowWidget);
	// }

	if (!NewGameState)
	{
		UE_LOG(LogTemp, Warning, TEXT("[LeaderBoardWidget] NewGameState is NULL"));
		return;
	}

	if (!PlayerBox)
	{
		UE_LOG(LogTemp, Warning, TEXT("[LeaderBoardWidget] PlayerBox is NULL"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[LeaderBoardWidget] Binding OnLeaderboardMapUpdated delegate"));
	NewGameState->OnLeaderboardMapUpdated.AddUObject(this, &UPCLeaderBoardWidget::SetupLeaderBoard);

	UE_LOG(LogTemp, Log, TEXT("[LeaderBoardWidget] Leaderboard entries count = %d"), NewGameState->Leaderboard.Num());

	for (const auto& PlayerRow : NewGameState->Leaderboard)
	{
		auto PlayerRowWidget = CreateWidget<UPCPlayerRowWidget>(GetWorld(), PlayerRowWidgetClass);
		if (!PlayerRowWidget)
		{
			UE_LOG(LogTemp, Warning, TEXT("[LeaderBoardWidget] Failed to create PlayerRowWidget for UserId=%s"),
				*PlayerRow.LocalUserId);
			continue;
		}

		PlayerRowWidget->SetupPlayerInfo(PlayerRow.LocalUserId, PlayerRow.Hp, PlayerRow.CharacterTag);
		PlayerMap.Add(PlayerRow.LocalUserId, PlayerRowWidget);
		PlayerBox->AddChild(PlayerRowWidget);

		UE_LOG(LogTemp, Log, TEXT("[LeaderBoardWidget] Added row: UserId=%s, Hp=%f, Tag=%s"),
			*PlayerRow.LocalUserId,	PlayerRow.Hp,
			*PlayerRow.CharacterTag.ToString());
	}
}

void UPCLeaderBoardWidget::SetupLeaderBoard(const TMap<FString, FPlayerStandingRow>& NewMap)
{
	TArray<TPair<int32, UPCPlayerRowWidget*>> RankArray;
	
	for (const auto& PlayerRow : NewMap)
	{
		if (auto PlayerRowWidget = PlayerMap.FindRef(PlayerRow.Key))
		{
			PlayerRowWidget->UpdatePlayerHP(PlayerRow.Value.Hp);
			RankArray.Add({PlayerRow.Value.LiveRank, PlayerRowWidget});
		}
	}

	RankArray.Sort([](const auto& A, const auto& B)
	{
		return A.Key < B.Key;
	});

	PlayerBox->ClearChildren();
	for (const auto& RankPair : RankArray)
	{
		if (RankPair.Value)
		{
			PlayerBox->AddChild(RankPair.Value);
		}
	}
}


