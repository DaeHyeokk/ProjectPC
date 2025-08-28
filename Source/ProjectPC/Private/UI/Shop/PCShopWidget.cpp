// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Shop/PCShopWidget.h"

#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "Components/TextBlock.h"
#include "Controller/Player/PCCombatPlayerController.h"

#include "GameFramework/GameState/PCCombatGameState.h"
#include "Shop/PCShopManager.h"
#include "UI/Shop/PCUnitSlotWidget.h"


bool UPCShopWidget::Initialize()
{
	if (!Super::Initialize()) return false;

	if (!Btn_BuyXP) return false;
	Btn_BuyXP->OnClicked.AddDynamic(this, &UPCShopWidget::OnClickedBuyXP);
	if (!Btn_Reroll) return false;
	Btn_Reroll->OnClicked.AddDynamic(this, &UPCShopWidget::OnClickedReroll);

	return true;
}

void UPCShopWidget::OnClickedBuyXP()
{
}

void UPCShopWidget::OnClickedReroll()
{
	if (auto PC = Cast<APCCombatPlayerController>(GetOwningPlayer()))
	{
		PC->ShopRequest_ShopRefresh();
		SetupShopSlots();
	}
}

void UPCShopWidget::OpenMenu()
{
	SetupShopSlots();
	this->AddToViewport();
}

void UPCShopWidget::CloseMenu()
{
	this->RemoveFromParent();
}

void UPCShopWidget::SetupShopSlots()
{
	if (!ShopBox) return;
	ShopBox->ClearChildren();
	
	auto GS = GetWorld()->GetGameState<APCCombatGameState>();
	if (!GS) return;
	
	const auto& ShopSlots = GS->GetShopManager()->GetShopSlots();

	// GameState에서 받아온 슬롯 정보로 UnitSlotWidget Child 생성
	for (const FPCShopUnitData& UnitData : ShopSlots)
	{
		auto UnitSlotWidget = CreateWidget<UPCUnitSlotWidget>(GetWorld(), UnitSlotWidgetClass);
		if (!UnitSlotWidget) return;
		
		UnitSlotWidget->Setup(UnitData);
		ShopBox->AddChild(UnitSlotWidget);
	}

	// 코스트 확률 정보 Text 세팅
	auto CostProbabilities = GS->GetCostProbabilities();
	TArray<UTextBlock*> CostTextBlocks = { Cost1, Cost2, Cost3, Cost4, Cost5 };
	for (int32 i = 0; i < CostTextBlocks.Num(); ++i)
	{
		int32 Percent = FMath::RoundToInt(CostProbabilities[i] * 100);
		FString Text = FString::Printf(TEXT("%d%%"), Percent);
		CostTextBlocks[i]->SetText(FText::FromString(Text));
	}
}
