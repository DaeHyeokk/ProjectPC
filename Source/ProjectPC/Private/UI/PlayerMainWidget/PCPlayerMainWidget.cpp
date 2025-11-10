// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PlayerMainWidget/PCPlayerMainWidget.h"

#include "UI/Item/PCPlayerInventoryWidget.h"
#include "UI/PlayerMainWidget/PCGameStateWidget.h"
#include "UI/PlayerMainWidget/PCLeaderBoardWidget.h"
#include "UI/Shop/PCShopWidget.h"
#include "UI/Unit/PCHeroStatusHoverPanel.h"


void UPCPlayerMainWidget::InitAndBind(APCCombatGameState* PCCombatGameState)
{
	if (!W_GameStateWidget)
		return;
	W_GameStateWidget->GameStateBinding();
	
	if (!W_LeaderBoardWidget && !PCCombatGameState)
		return;
	W_LeaderBoardWidget->BindToGameState(PCCombatGameState);

	if (!W_HeroStatus)
		return;
	W_HeroStatus->Init();
	W_HeroStatus->SetVisibility(ESlateVisibility::Hidden);

	if (!W_ShopWidget) return;

	W_ShopWidget->SetVisibility(ESlateVisibility::Hidden);
}

void UPCPlayerMainWidget::SetShopWidgetVisible(bool bVisible)
{
	if (!W_ShopWidget) return;

	if (bVisible)
	{
		W_ShopWidget->SetVisibility(ESlateVisibility::Visible);
		return;
	}

	W_ShopWidget->SetVisibility(ESlateVisibility::Hidden);
}

void UPCPlayerMainWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	SetVisibility(ESlateVisibility::Hidden);
}
