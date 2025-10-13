// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PlayerMainWidget/PCPlayerMainWidget.h"

#include "GameFramework/PlayerState/PCPlayerState.h"
#include "UI/PlayerMainWidget/PCGameStateWidget.h"
#include "UI/PlayerMainWidget/PCLeaderBoardWidget.h"
#include "UI/PlayerMainWidget/PCPlayerRowWidget.h"
#include "UI/Shop/PCShopWidget.h"


void UPCPlayerMainWidget::InitAndBind(APCCombatGameState* PCCombatGameState)
{
	if (!W_GameStateWidget)
		return;
	W_GameStateWidget->GameStateBinding();
	
	if (!W_LeaderBoardWidget && !PCCombatGameState)
		return;
	W_LeaderBoardWidget->BindToGameState(PCCombatGameState);
	
}
