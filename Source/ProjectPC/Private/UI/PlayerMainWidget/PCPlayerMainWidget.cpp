// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PlayerMainWidget/PCPlayerMainWidget.h"

#include "Component/PCSynergyComponent.h"
#include "Controller/Player/PCCombatPlayerController.h"
#include "GameFramework/PlayerState/PCPlayerState.h"
#include "UI/Item/PCPlayerInventoryWidget.h"
#include "UI/PlayerMainWidget/PCGameStateWidget.h"
#include "UI/PlayerMainWidget/PCLeaderBoardWidget.h"
#include "UI/Shop/PCShopWidget.h"
#include "UI/Synerge/PCSynergyPanelWidget.h"
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
	W_HeroStatus->SetVisibility(ESlateVisibility::Hidden);

	if (!W_ShopWidget) return;

	W_ShopWidget->SetVisibility(ESlateVisibility::Hidden);

	if (!W_SynergyWidget) return;

	W_SynergyWidget->SetSynergyComponent(ResolveSynergyComp());
	
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

UPCSynergyComponent* UPCPlayerMainWidget::ResolveSynergyComp()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		if (APlayerState* PS = PC->PlayerState)
		{
			if (APCPlayerState* PCPS = Cast<APCPlayerState>(PS))
			{
				return PCPS->FindComponentByClass<UPCSynergyComponent>();
			}
		}
	}

	return nullptr;
}
