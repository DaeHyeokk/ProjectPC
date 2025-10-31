// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PlayerMainWidget/PCLeaderBoardWidget.h"

#include "Components/VerticalBox.h"
#include "GameFramework/GameState/PCCombatGameState.h"
#include "UI/PlayerMainWidget/PCPlayerRowWidget.h"


void UPCLeaderBoardWidget::BindToGameState(APCCombatGameState* NewGameState)
{
	if (!NewGameState || !PlayerBox) return;
	
	NewGameState->OnPlayerRankingChanged.AddUObject(this, &UPCLeaderBoardWidget::SetupLeaderBoard);

	for (auto Player : NewGameState->GetPlayerRanking())
	{
		auto PlayerRowWidget = CreateWidget<UPCPlayerRowWidget>(GetWorld(), PlayerRowWidgetClass);
		if (!PlayerRowWidget) continue;

		for (auto PS : NewGameState->PlayerArray)
		{
			if (auto PCPS = Cast<APCPlayerState>(PS))
			{
				if (PCPS->LocalUserId == Player)
				{
					PlayerRowWidget->SetupPlayerInfo(PCPS);
					PlayerMap.Add(Player, PlayerRowWidget);
					PlayerBox->AddChild(PlayerRowWidget);
				}
			}
		}
	}
}

void UPCLeaderBoardWidget::SetupLeaderBoard(const TArray<FString>& NewPlayerRanking) const
{
	TArray<UPCPlayerRowWidget*> RankArray;
	
	for (const auto Player : NewPlayerRanking)
	{
		if (PlayerMap.Contains(Player))
		{
			if (auto PlayerRowWidget = PlayerMap.FindRef(Player))
			{
				RankArray.Add(PlayerRowWidget);
			}
		}
	}
	
	PlayerBox->ClearChildren();
	for (const auto& Rank : RankArray)
	{
		if (Rank)
		{
			PlayerBox->AddChild(Rank);
			Rank->SetWinningFlame();
		}
	}
}

void UPCLeaderBoardWidget::ExpandPlayerRowWidget(FString PlayerName)
{
	for (auto Player : PlayerMap)
	{
		if (Player.Key == PlayerName && IsValid(Player.Value))
		{
			Player.Value->ExpandRenderSize();
		}
		else
		{
			Player.Value->RestoreRenderSize();
		}
	}
}
