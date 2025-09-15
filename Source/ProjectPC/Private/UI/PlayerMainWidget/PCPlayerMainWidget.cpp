// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PlayerMainWidget/PCPlayerMainWidget.h"

#include "GameFramework/PlayerState/PCPlayerState.h"
#include "UI/PlayerMainWidget/PCGameStateWidget.h"
#include "UI/Shop/PCShopWidget.h"

void UPCPlayerMainWidget::InitAndBind()
{
	if (!W_GameStateWidget)
		return;
	W_GameStateWidget->GameStateBinding();
	
}
