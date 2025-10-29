// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Shop/PCShopWidget.h"

#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "Components/Overlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/WidgetSwitcher.h"
#include "GameplayEffectTypes.h"

#include "UI/Shop/PCUnitSlotWidget.h"
#include "GameFramework/GameState/PCCombatGameState.h"
#include "GameFramework/PlayerState/PCPlayerState.h"
#include "Controller/Player/PCCombatPlayerController.h"
#include "AbilitySystem/Player/AttributeSet/PCPlayerAttributeSet.h"
#include "Shop/PCShopManager.h"


bool UPCShopWidget::Initialize()
{
	if (!Super::Initialize()) return false;

	if (!Btn_BuyXP) return false;
	Btn_BuyXP->OnClicked.AddDynamic(this, &UPCShopWidget::OnClickedBuyXP);
	if (!Btn_Reroll) return false;
	Btn_Reroll->OnClicked.AddDynamic(this, &UPCShopWidget::OnClickedReroll);
	if (!Btn_ShopLock) return false;
	Btn_ShopLock->OnClicked.AddDynamic(this, &UPCShopWidget::OnClickedShopLock);

	return true;
}

void UPCShopWidget::BindToPlayerState(APCPlayerState* NewPlayerState)
{
	if (!NewPlayerState) return;
	CachedPlayerState = NewPlayerState;

	CachedPlayerState->OnShopSlotsUpdated.AddUObject(this, &UPCShopWidget::SetupShopSlots);
	CachedPlayerState->OnWinningStreakUpdated.AddUObject(this, &UPCShopWidget::OnPlayerWinningStreakChanged);
	
	SetupShopSlots();
	SetupPlayerInfo();

	if (auto ASC = CachedPlayerState->GetAbilitySystemComponent())
	{
		if (auto AttributeSet = CachedPlayerState->GetAttributeSet())
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

void UPCShopWidget::InitWithPC(APCCombatPlayerController* InPC)
{
	CachedController = InPC;
}

void UPCShopWidget::OpenMenu()
{
	AddToViewport(100);
	SetVisibility(ESlateVisibility::Hidden);
}

void UPCShopWidget::CloseMenu()
{
	RemoveFromParent();
}

void UPCShopWidget::SetupShopSlots()
{
	if (!ShopBox)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ShopWidget] ShopBox is NULL"));
		return;
	}

	if (!CachedPlayerState)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ShopWidget] CachedPlayerState is NULL"));
		return;
	}

	ShopBox->ClearChildren();
	UE_LOG(LogTemp, Log, TEXT("[ShopWidget] Cleared children."));

	const auto& ShopSlots = CachedPlayerState->GetShopSlots();
	UE_LOG(LogTemp, Log, TEXT("[ShopWidget] ShopSlots count = %d"), ShopSlots.Num());

	// GameState에서 받아온 슬롯 정보로 UnitSlotWidget Child 생성
	int32 Index = 0;
	for (const FPCShopUnitData& UnitData : ShopSlots)
	{
		auto UnitSlotWidget = CreateWidget<UPCUnitSlotWidget>(GetWorld(), UnitSlotWidgetClass);
		if (!UnitSlotWidget)
		{
			UE_LOG(LogTemp, Warning, TEXT("[ShopWidget] Failed to create UnitSlotWidget at Index=%d"), Index);
			continue;
		}

		UnitSlotWidget->Setup(UnitData, true, Index);
		ShopBox->AddChild(UnitSlotWidget);

		UE_LOG(LogTemp, Log, TEXT("[ShopWidget] Added UnitSlotWidget Index=%d, UnitTag=%s"), 
			Index, *UnitData.UnitTag.ToString());

		++Index;
	}
}

void UPCShopWidget::SetupPlayerInfo()
{
	if (!GoldBalance || !Level || !XP || !XPBar) return;
	if (!CachedPlayerState) return;
	
	auto GS = GetWorld()->GetGameState<APCCombatGameState>();
	if (!GS) return;
	
	const auto AttributeSet = CachedPlayerState->GetAttributeSet();
	if (!AttributeSet) return;

	// 플레이어 정보 (레벨) 세팅
	PlayerLevel = static_cast<int32>(AttributeSet->GetPlayerLevel());
	auto LevelText = FString::Printf(TEXT("Lv.%d"), PlayerLevel);
	Level->SetText(FText::FromString(LevelText));

	// 플레이어 정보 (경험치) 세팅
	PlayerXP = static_cast<int32>(AttributeSet->GetPlayerXP());
	PlayerMaxXP = GS->GetMaxXP(PlayerLevel);
	auto XPText = FString::Printf(TEXT("%d/%d"), PlayerXP, PlayerMaxXP);
	XP->SetText(FText::FromString(XPText));
	
	if(!PlayerMaxXP == 0)
	{
		XPBar->SetPercent(static_cast<float>(PlayerXP) / static_cast<float>(PlayerMaxXP));
	}
	
	// 플레이어 정보 (골드) 세팅
	PlayerGold = static_cast<int32>(AttributeSet->GetPlayerGold());
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
	if (CachedController.IsValid())
	{
		CachedController->ShopRequest_BuyXP();
	}
	
}

void UPCShopWidget::OnClickedReroll()
{	
	if (CachedController.IsValid())
	{
		CachedController->ShopRequest_ShopRefresh(2);
	}
	
}

void UPCShopWidget::OnClickedShopLock()
{
	if (!Img_ShopLock || !ShopLock || !ShopUnlock || !CachedController.IsValid()) return;
	
	const auto& CurrentBrush = Img_ShopLock->GetBrush();
	auto CurrentResource = CurrentBrush.GetResourceObject();

	if (CurrentResource && CurrentResource == ShopLock)
	{
		Img_ShopLock->SetBrushFromTexture(ShopUnlock);
		CachedController->ShopRequest_ShopLock(false);
	}
	else
	{
		Img_ShopLock->SetBrushFromTexture(ShopLock);
		CachedController->ShopRequest_ShopLock(true);
	}
}

void UPCShopWidget::OnPlayerLevelChanged(const FOnAttributeChangeData& Data)
{
	if (!XP || !XPBar || !Level) return;

	auto GS = GetWorld()->GetGameState<APCCombatGameState>();
	if (!GS) return;

	PlayerLevel = static_cast<int32>(Data.NewValue);
	auto LevelText = FString::Printf(TEXT("Lv.%d"), PlayerLevel);
	Level->SetText(FText::FromString(LevelText));

	auto CostProbabilities = GS->GetShopManager()->GetCostProbabilities(PlayerLevel);
	TArray<UTextBlock*> CostTextBlocks = { Cost1, Cost2, Cost3, Cost4, Cost5 };
	for (int32 i = 0; i < CostTextBlocks.Num(); ++i)
	{
		int32 Percent = FMath::RoundToInt(CostProbabilities[i] * 100);
		FString Text = FString::Printf(TEXT("%d%%"), Percent);
		CostTextBlocks[i]->SetText(FText::FromString(Text));
	}

	PlayerMaxXP = GS->GetMaxXP(PlayerLevel);
	FString XPText = FString::Printf(TEXT("%d/%d"), PlayerXP, PlayerMaxXP);
	XP->SetText(FText::FromString(XPText));

	if(!PlayerMaxXP == 0)
	{
		XPBar->SetPercent(static_cast<float>(PlayerXP) / static_cast<float>(PlayerMaxXP));	
	}
}

void UPCShopWidget::OnPlayerXPChanged(const FOnAttributeChangeData& Data)
{
	if (!XP || !XPBar) return;
	
	PlayerXP = static_cast<int32>(Data.NewValue);
	
	FString XPText = FString::Printf(TEXT("%d/%d"), PlayerXP, PlayerMaxXP);
	XP->SetText(FText::FromString(XPText));

	if(!PlayerMaxXP == 0)
	{
		XPBar->SetPercent(static_cast<float>(PlayerXP) / static_cast<float>(PlayerMaxXP));
	}
}

void UPCShopWidget::OnPlayerGoldChanged(const FOnAttributeChangeData& Data)
{
	if (!GoldBalance || !ShopBox) return;
	
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

void UPCShopWidget::OnPlayerWinningStreakChanged()
{
	if (!WinningStreak || !Img_WinningStreak || !Winning || !Losing) return;
	if (!CachedPlayerState) return;

	auto WinningCount = CachedPlayerState->GetPlayerWinningStreak();
	if (WinningCount > 0)
	{
		Img_WinningStreak->SetBrushFromTexture(Winning);
		WinningStreak->SetText(FText::AsNumber(WinningCount));
	}
	else
	{
		Img_WinningStreak->SetBrushFromTexture(Losing);
		WinningStreak->SetText(FText::AsNumber(-WinningCount));
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
	if (!WidgetSwitcher || !PlayerShopBox || !SellBox) return false;
	
	const FGeometry ViewportGeo = UWidgetLayoutLibrary::GetViewportWidgetGeometry(GetWorld());
	const FVector2D AbsolutePos = ViewportGeo.LocalToAbsolute(Point);

	if (WidgetSwitcher->GetActiveWidget() == PlayerShopBox)
	{
		const FGeometry& PlayerShopBoxGeo = PlayerShopBox->GetCachedGeometry();
		return PlayerShopBoxGeo.IsUnderLocation(AbsolutePos);
	}
	
	const FGeometry& SellBoxGeo = SellBox->GetCachedGeometry();
	return SellBoxGeo.IsUnderLocation(AbsolutePos);
}