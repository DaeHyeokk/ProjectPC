// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PlayerMainWidget/PCLeaderBoardWidget.h"

#include "Components/VerticalBox.h"
#include "GameFramework/GameState/PCCombatGameState.h"
#include "UI/PlayerMainWidget/PCPlayerRowWidget.h"


void UPCLeaderBoardWidget::BindToGameState(APCCombatGameState* NewGameState)
{
	if (!NewGameState || !PlayerBox) return;

	// 플레이어 순위 변화 구독
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
					// PlayerRowWidget에 PlayerState 바인딩
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

	// FString Key값으로 캐싱된 PlayerMap의 Value(PlayerRowWidget)를 찾아 순위별로 RankArray에 정렬
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

	// 현재 PlayerBox의 Child를 초기화 해주고 순위별로 다시 Child 추가
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
	// 현재 화면에 보이는 플레이어 위젯 강조
	for (auto Player : PlayerMap)
	{
		if (Player.Key == PlayerName && IsValid(Player.Value))
		{
			// PlayerName과 일치하는 PlayerRowWidget 크기 확대
			Player.Value->ExpandRenderSize();
		}
		else
		{
			// PlayerName과 일치하지 않는 PlayerRowWidget 크기 원복
			Player.Value->RestoreRenderSize();
		}
	}
}
