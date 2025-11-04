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

	for (int32 i = 0; i < 5; ++i)
	{
		auto UnitSlotWidget = CreateWidget<UPCUnitSlotWidget>(GetWorld(), UnitSlotWidgetClass);
		if (!UnitSlotWidget) continue;

		UnitSlotWidgets.Add(UnitSlotWidget);
	}

	return true;
}

void UPCShopWidget::NativeDestruct()
{
	// 델리게이트 언바인딩
	if (CachedPlayerState)
	{
		if (auto ASC = CachedPlayerState->GetAbilitySystemComponent())
		{
			if (auto AttributeSet = CachedPlayerState->GetAttributeSet())
			{
				ASC->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetPlayerLevelAttribute())
					.RemoveAll(this);
				ASC->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetPlayerXPAttribute())
					.RemoveAll(this);
				ASC->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetPlayerGoldAttribute())
					.RemoveAll(this);
			}
		}

		CachedPlayerState->OnShopSlotsUpdated.RemoveAll(this);
		CachedPlayerState->OnWinningStreakUpdated.RemoveAll(this);
	}

	Super::NativeDestruct();
}

void UPCShopWidget::BindToPlayerState(APCPlayerState* NewPlayerState)
{
	if (!NewPlayerState) return;
	CachedPlayerState = NewPlayerState;

	// 플레이어 상점 슬롯 변화, 연승 기록 구독
	CachedPlayerState->OnShopSlotsUpdated.AddUObject(this, &UPCShopWidget::SetupShopSlots);
	CachedPlayerState->OnWinningStreakUpdated.AddUObject(this, &UPCShopWidget::OnPlayerWinningStreakChanged);
	
	SetupShopSlots();
	SetupPlayerInfo();

	// 플레이어 어트리뷰트 (Level, XP, Gold) 구독
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

void UPCShopWidget::SetupShopSlots()
{
	if (!ShopBox || !CachedPlayerState)
		return;

	ShopBox->ClearChildren();

	const auto& ShopSlots = CachedPlayerState->GetShopSlots();

	// GameState에서 받아온 슬롯 정보로 UnitSlotWidget Child 생성
	int32 Index = 0;
	for (const FPCShopUnitData& UnitData : ShopSlots)
	{
		if (UnitSlotWidgets.IsValidIndex(Index) && UnitSlotWidgets[Index])
		{
			UnitSlotWidgets[Index]->Setup(UnitData, true, Index);
			UnitSlotWidgets[Index]->SetSlotHidden(false);
			ShopBox->AddChild(UnitSlotWidgets[Index]);
			++Index;
		}
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
	
	if(PlayerMaxXP > 0)
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
		// 상점 잠금 해제
		Img_ShopLock->SetBrushFromTexture(ShopUnlock);
		CachedController->ShopRequest_ShopLock(false);
	}
	else
	{
		// 상점 잠금
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
		if (UnitSlotWidgets.IsValidIndex(i) && UnitSlotWidgets[i])
		{
			UnitSlotWidgets[i]->SetupButton();
		}
	}
}

void UPCShopWidget::OnPlayerWinningStreakChanged(int32 NewWinningStreak)
{
	if (!WinningStreak || !Img_WinningStreak || !Winning || !Losing) return;
	if (!CachedPlayerState) return;

	if (NewWinningStreak > 0)
	{
		// 연승
		Img_WinningStreak->SetBrushFromTexture(Winning);
		WinningStreak->SetText(FText::AsNumber(NewWinningStreak));
	}
	else
	{
		// 연패
		Img_WinningStreak->SetBrushFromTexture(Losing);
		WinningStreak->SetText(FText::AsNumber(-NewWinningStreak));
	}
}

void UPCShopWidget::SetSlotHidden(int32 SlotIndex)
{
	if (UnitSlotWidgets.IsValidIndex(SlotIndex) && UnitSlotWidgets[SlotIndex])
	{
		UnitSlotWidgets[SlotIndex]->SetSlotHidden(true);
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