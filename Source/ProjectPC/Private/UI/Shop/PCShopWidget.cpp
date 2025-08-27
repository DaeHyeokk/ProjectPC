// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Shop/PCShopWidget.h"

#include "Components/Button.h"
#include "Components/HorizontalBox.h"

#include "GameFramework/GameState/PCCombatGameState.h"
#include "Shop/PCShopManager.h"
#include "UI/Shop/PCUnitSlotWidget.h"


UPCShopWidget::UPCShopWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

bool UPCShopWidget::Initialize()
{
	bool SuperSuccess = Super::Initialize();
	if (!SuperSuccess) return false;

	if (!Btn_BuyXP) return false;
	Btn_BuyXP->OnClicked.AddDynamic(this, &UPCShopWidget::OnClickedBuyXP);
	if (!Btn_Reroll) return false;
	Btn_Reroll->OnClicked.AddDynamic(this, &UPCShopWidget::OnClickedReroll);

	return true;
}

void UPCShopWidget::OnClickedBuyXP()
{
	UE_LOG(LogTemp, Warning, TEXT("Btn_BuyXP Clicked"));
}

void UPCShopWidget::OnClickedReroll()
{
	UE_LOG(LogTemp, Warning, TEXT("Btn_Reroll Clicked"));
}

void UPCShopWidget::OpenMenu()
{
	this->AddToViewport();
}

void UPCShopWidget::CloseMenu()
{
	this->RemoveFromParent();
}

void UPCShopWidget::SetupShopSlots()
{
	auto GS = GetWorld()->GetGameState<APCCombatGameState>();
	if (!GS) return;
	if (!ShopBox) return;

	auto ShopSlots = GS->ShopManager->GetShopSlots();
	ShopBox->ClearChildren();

	for (const FPCShopUnitData& UnitData : ShopSlots)
	{
		UE_LOG(LogTemp, Warning, TEXT("UnitName : %s"), *UnitData.UnitName.ToString());
		auto UnitSlotWidget = CreateWidget<UPCUnitSlotWidget>(GetWorld(), UnitSlotWidgetClass);
		if (!UnitSlotWidget) return;
		
		UnitSlotWidget->Setup(UnitData);
		ShopBox->AddChild(UnitSlotWidget);
	}
}
