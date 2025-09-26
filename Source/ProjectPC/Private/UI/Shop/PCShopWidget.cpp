// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Shop/PCShopWidget.h"

#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "Components/Overlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "GameplayEffectTypes.h"

#include "UI/Shop/PCUnitSlotWidget.h"
#include "GameFramework/GameState/PCCombatGameState.h"
#include "GameFramework/PlayerState/PCPlayerState.h"
#include "Controller/Player/PCCombatPlayerController.h"
#include "AbilitySystem/Player/AttributeSet/PCPlayerAttributeSet.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/WidgetSwitcher.h"
#include "Shop/PCShopManager.h"


bool UPCShopWidget::Initialize()
{
	if (!Super::Initialize()) return false;

	if (!Btn_BuyXP) return false;
	Btn_BuyXP->OnClicked.AddDynamic(this, &UPCShopWidget::OnClickedBuyXP);
	if (!Btn_Reroll) return false;
	Btn_Reroll->OnClicked.AddDynamic(this, &UPCShopWidget::OnClickedReroll);

	return true;
}

void UPCShopWidget::BindToPlayerState(APCPlayerState* NewPlayerState)
{
	if (!NewPlayerState) return;

	NewPlayerState->OnShopSlotsUpdated.AddLambda([this, NewPlayerState]()
	{
		SetupShopSlots();
	});
	
	SetupShopSlots();
	SetupPlayerInfo();

	if (auto ASC = NewPlayerState->GetAbilitySystemComponent())
	{
		if (auto AttributeSet = NewPlayerState->GetAttributeSet())
		{
			ASC->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetPlayerLevelAttribute())
			.AddUObject(this, &UPCShopWidget::OnPlayerLevelChanged);
			ASC->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetPlayerXPAttribute())
			.AddUObject(this, &UPCShopWidget::OnPlayerXPChanged);
			ASC->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetPlayerGoldAttribute())
			.AddUObject(this, &UPCShopWidget::OnPlayerGoldChanged);
		}
	}
}

void UPCShopWidget::OpenMenu()
{
	this->AddToViewport(100);
	this->SetVisibility(ESlateVisibility::Hidden);
}

void UPCShopWidget::CloseMenu()
{
	this->RemoveFromParent();
}

void UPCShopWidget::SetupShopSlots()
{
	if (!ShopBox) return;
	ShopBox->ClearChildren();
	
	auto PS = GetOwningPlayer()->GetPlayerState<APCPlayerState>();
	if (!PS) return;
	
	const auto& ShopSlots = PS->GetShopSlots();

	// GameState에서 받아온 슬롯 정보로 UnitSlotWidget Child 생성
	int32 Index = 0;
	for (const FPCShopUnitData& UnitData : ShopSlots)
	{
		auto UnitSlotWidget = CreateWidget<UPCUnitSlotWidget>(GetWorld(), UnitSlotWidgetClass);
		if (!UnitSlotWidget) return;
		
		UnitSlotWidget->Setup(UnitData, Index);
		ShopBox->AddChild(UnitSlotWidget);

		++Index;
	}
}

void UPCShopWidget::SetupPlayerInfo()
{
	if (!GoldBalance || !Level || !XP || !XPBar) return;

	auto GS = GetWorld()->GetGameState<APCCombatGameState>();
	if (!GS) return;
	auto PS = GetOwningPlayer()->GetPlayerState<APCPlayerState>();
	if (!PS) return;
	auto AttributeSet = PS->GetAttributeSet();
	if (!AttributeSet) return;

	// 플레이어 정보 (레벨) 세팅
	auto PlayerLevel = static_cast<int32>(AttributeSet->GetPlayerLevel());
	auto LevelText = FString::Printf(TEXT("Lv. %d"), PlayerLevel);
	Level->SetText(FText::FromString(LevelText));

	// 플레이어 정보 (경험치) 세팅
	auto PlayerXP = static_cast<int32>(AttributeSet->GetPlayerXP());
	auto PlayerMaxXP = GS->GetMaxXP(PlayerLevel);
	auto XPText = FString::Printf(TEXT("%d/%d"), PlayerXP, PlayerMaxXP);
	XP->SetText(FText::FromString(XPText));
	
	if(!PlayerMaxXP == 0)
	{
		XPBar->SetPercent(PlayerXP / PlayerMaxXP);
	}
	

	// 플레이어 정보 (골드) 세팅
	auto PlayerGold = static_cast<int32>(AttributeSet->GetPlayerGold());
	GoldBalance->SetText(FText::AsNumber(PlayerGold));
	
	// 코스트 확률 정보 Text 세팅
	auto CostProbabilities = GS->GetShopManager()->GetCostProbabilities(PlayerLevel);
	TArray<UTextBlock*> CostTextBlocks = { Cost1, Cost2, Cost3, Cost4, Cost5 };
	for (int32 i = 0; i < CostTextBlocks.Num(); ++i)
	{
		int32 Percent = FMath::RoundToInt(CostProbabilities[i] * 100);
		FString Text = FString::Printf(TEXT("%d%%"), Percent);
		CostTextBlocks[i]->SetText(FText::FromString(Text));
	}
}

void UPCShopWidget::OnClickedBuyXP()
{
	if (auto PC = Cast<APCCombatPlayerController>(GetOwningPlayer()))
	{
		PC->ShopRequest_BuyXP();
	}
}

void UPCShopWidget::OnClickedReroll()
{
	if (auto PC = Cast<APCCombatPlayerController>(GetOwningPlayer()))
	{
		PC->ShopRequest_ShopRefresh(2);
	}
}

void UPCShopWidget::OnPlayerLevelChanged(const FOnAttributeChangeData& Data)
{
	auto GS = GetWorld()->GetGameState<APCCombatGameState>();
	if (!GS || !Level) return;
	
	auto LevelText = FString::Printf(TEXT("Lv. %d"), static_cast<int32>(Data.NewValue));
	Level->SetText(FText::FromString(LevelText));

	auto CostProbabilities = GS->GetShopManager()->GetCostProbabilities(static_cast<int32>(Data.NewValue));
	TArray<UTextBlock*> CostTextBlocks = { Cost1, Cost2, Cost3, Cost4, Cost5 };
	for (int32 i = 0; i < CostTextBlocks.Num(); ++i)
	{
		int32 Percent = FMath::RoundToInt(CostProbabilities[i] * 100);
		FString Text = FString::Printf(TEXT("%d%%"), Percent);
		CostTextBlocks[i]->SetText(FText::FromString(Text));
	}
}

void UPCShopWidget::OnPlayerXPChanged(const FOnAttributeChangeData& Data)
{
	auto GS = GetWorld()->GetGameState<APCCombatGameState>();
	if (!GS) return;
	
	auto PS = GetOwningPlayer()->GetPlayerState<APCPlayerState>();
	if (!PS || !XP || !XPBar) return;

	auto AttributeSet = PS->GetAttributeSet();
	if (!AttributeSet) return;

	auto PlayerLevel = static_cast<int32>(AttributeSet->GetPlayerLevel());
	int32 MaxXP = GS->GetMaxXP(PlayerLevel);
	FString XPText = FString::Printf(TEXT("%d/%d"), static_cast<int32>(Data.NewValue), MaxXP);
	XP->SetText(FText::FromString(XPText));

	XPBar->SetPercent(Data.NewValue / MaxXP);
}

void UPCShopWidget::OnPlayerGoldChanged(const FOnAttributeChangeData& Data)
{
	if (!GoldBalance || !ShopBox) return;
	
	auto PS = GetOwningPlayer()->GetPlayerState<APCPlayerState>();
	if (!PS) return;
	
	GoldBalance->SetText(FText::AsNumber(static_cast<int32>(Data.NewValue)));
	
	// 골드가 바뀔 때마다 상점 슬롯도 업데이트 (바뀐 골드로 구매 불가능한 유닛 판별)
	for (int32 i = 0; i < ShopBox->GetChildrenCount(); ++i)
	{
		if (auto UnitSlotWidget = Cast<UPCUnitSlotWidget>(ShopBox->GetChildAt(i)))
		{
			UnitSlotWidget->SetupButton();
		}
	}
}

void UPCShopWidget::SetSlotHidden(int32 SlotIndex)
{
	if (auto SelectedSlot = Cast<UPCUnitSlotWidget>(ShopBox->GetChildAt(SlotIndex)))
	{
		SelectedSlot->SetSlotHidden(true);
	}
}

void UPCShopWidget::ShowPlayerShopBox() const
{
	if (!WidgetSwitcher) return;

	if (WidgetSwitcher->GetActiveWidget() == SellBox)
	{
		WidgetSwitcher->SetActiveWidget(PlayerShopBox);
	}
}

void UPCShopWidget::ShowSellBox() const
{
	if (!WidgetSwitcher) return;

	if (WidgetSwitcher->GetActiveWidget() == PlayerShopBox)
	{
		WidgetSwitcher->SetActiveWidget(SellBox);
	}
}

bool UPCShopWidget::IsScreenPointInSellBox(const FVector2D& Point) const
{
	if (!WidgetSwitcher)
		return false;
	
	const FGeometry ViewportGeo = UWidgetLayoutLibrary::GetViewportWidgetGeometry(GetWorld());
	const FVector2D AbsolutePos = ViewportGeo.LocalToAbsolute(Point);
	
	//const FGeometry& GM = WidgetSwitcher->GetCachedGeometry();
	const FGeometry& GM = SellBox->GetCachedGeometry();

	return GM.IsUnderLocation(AbsolutePos);
}
