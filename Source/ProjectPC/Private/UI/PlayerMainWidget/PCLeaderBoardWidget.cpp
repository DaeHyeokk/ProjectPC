// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PlayerMainWidget/PCLeaderBoardWidget.h"

#include "Components/VerticalBox.h"
#include "GameFramework/GameState/PCCombatGameState.h"
#include "UI/PlayerMainWidget/PCPlayerRowWidget.h"


void UPCLeaderBoardWidget::BindToGameState(APCCombatGameState* NewGameState)
{
	if (!NewGameState || !PlayerBox) return;
	
	NewGameState->FindPlayerState.AddUObject(this, &UPCLeaderBoardWidget::SetupLeaderBoard);
	
	for (APCPlayerState* Player : NewGameState->GetPlayerStates())
	{
		auto PlayerRowWidget = CreateWidget<UPCPlayerRowWidget>(GetWorld(), PlayerRowWidgetClass);
		if (!PlayerRowWidget || !Player) continue;
	
		PlayerRowWidget->SetupPlayerInfo(Player);
		// PlayerMap.Add(PlayerRow.LocalUserId, PlayerRowWidget);
		PlayerMap.Add(TWeakObjectPtr<APCPlayerState>(Player), PlayerRowWidget);
		PlayerBox->AddChild(PlayerRowWidget);
	}
}

// void UPCLeaderBoardWidget::SetupLeaderBoard(const TMap<FString, FPlayerStandingRow>& NewMap) const
// {
// 	TArray<UPCPlayerRowWidget*> RankArray;
// 	
// 	for (const auto& PlayerRow : NewMap)
// 	{
// 		if (auto PlayerRowWidget = PlayerMap.FindRef(PlayerRow.Key))
// 		{
// 			PlayerRowWidget->UpdatePlayerHP(PlayerRow.Value.Hp);
// 			RankArray.Add(PlayerRowWidget);
// 		}
// 	}
// 	
// 	PlayerBox->ClearChildren();
// 	for (const auto& Rank : RankArray)
// 	{
// 		if (Rank)
// 		{
// 			PlayerBox->AddChild(Rank);
// 		}
// 	}
// }

void UPCLeaderBoardWidget::SetupLeaderBoard(const TArray<APCPlayerState*>& NewPlayerArray) const
{
	TArray<UPCPlayerRowWidget*> RankArray;
	
	for (const auto Player : NewPlayerArray)
	{
		TWeakObjectPtr<APCPlayerState> PlayerKey(Player);
		if (PlayerMap.Contains(PlayerKey))
		{
			if (auto PlayerRowWidget = PlayerMap.FindRef(PlayerKey))
			{
				PlayerRowWidget->UpdatePlayerHP();
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
		}
	}
}
