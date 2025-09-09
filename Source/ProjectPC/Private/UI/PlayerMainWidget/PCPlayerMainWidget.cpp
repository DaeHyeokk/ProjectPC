// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PlayerMainWidget/PCPlayerMainWidget.h"

#include "UI/PlayerMainWidget/PCGameStateWidget.h"
#include "UI/Shop/PCShopWidget.h"

void UPCPlayerMainWidget::InitAndBind()
{
	if (!W_GameStateWidget)
		return;
	W_GameStateWidget->GameStateBinding();
}

void UPCPlayerMainWidget::HideWidget()
{
	if (!W_ShopWidget)
		return;
	W_ShopWidget->SetVisibility(ESlateVisibility::Hidden);
}

void UPCPlayerMainWidget::ShowWidget()
{
	if (!W_ShopWidget)
		return;
	W_ShopWidget->SetVisibility(ESlateVisibility::Visible);
}
