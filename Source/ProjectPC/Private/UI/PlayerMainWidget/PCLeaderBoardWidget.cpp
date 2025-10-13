// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PlayerMainWidget/PCLeaderBoardWidget.h"

#include "Components/VerticalBox.h"
#include "GameFramework/GameState/PCCombatGameState.h"
#include "UI/PlayerMainWidget/PCPlayerRowWidget.h"


void UPCLeaderBoardWidget::BindToGameState(APCCombatGameState* NewGameState)
{
	if (!NewGameState || !PlayerBox) return;
	// CachedGameState = NewGameState;

	NewGameState->OnLeaderboardMapUpdated.AddUObject(this, &UPCLeaderBoardWidget::SetupLeaderBoard);
	
	for (const auto& PlayerRow : NewGameState->Leaderboard)
	{
		auto PlayerRowWidget = CreateWidget<UPCPlayerRowWidget>(GetWorld(), PlayerRowWidgetClass);
		if (!PlayerRowWidget) continue;

		PlayerRowWidget->SetupPlayerInfo(PlayerRow.LocalUserId, PlayerRow.Hp, PlayerRow.CharacterTag);
		PlayerMap.Add(PlayerRow.LocalUserId, PlayerRowWidget);
		PlayerBox->AddChild(PlayerRowWidget);
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
	
	// for (int32 i = 0; i < RankArray.Num(); ++i)
	// {
	// 	auto PlayerRowWidget = RankArray[i].Value;
	// 	if (PlayerRowWidget && PlayerBox->GetChildAt(i) != PlayerRowWidget)
	// 	{
	// 		PlayerBox->RemoveChild(PlayerRowWidget);
	// 		PlayerBox->InsertChildAt(i, PlayerRowWidget);
	// 	}
	// }
}


