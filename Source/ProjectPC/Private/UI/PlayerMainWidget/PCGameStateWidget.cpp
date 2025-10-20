// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PlayerMainWidget/PCGameStateWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/ProgressBar.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "DataAsset/UI/PCWidgetIconData.h"
#include "GameFramework/GameState/PCCombatGameState.h"
#include "Kismet/GameplayStatics.h"
#include "UI/PlayerMainWidget/PCRoundCellWidget.h"


void UPCGameStateWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	GameStateBinding();

}

void UPCGameStateWidget::NativeDestruct()
{
	if (PCGameState.IsValid())
	{
		if (RepHandle.IsValid())
		{
			PCGameState->OnStageRuntimeChanged.Remove(RepHandle);
		}

		if (LayOutHandle.IsValid())
		{
			PCGameState->OnRoundsLayoutChanged.Remove(LayOutHandle);
		}
		
	}
	GetWorld()->GetTimerManager().ClearTimer(TickHandle);
	Super::NativeDestruct();
}

void UPCGameStateWidget::GameStateBinding()
{
	
	PCGameState = Cast<APCCombatGameState>(UGameplayStatics::GetGameState(this));
	if (PCGameState.IsValid())
	{
		RepHandle = PCGameState->OnStageRuntimeChanged.AddUObject(this, &UPCGameStateWidget::ReFreshStatic);
		ReFreshStatic();
	}

	LayOutHandle = PCGameState->OnRoundsLayoutChanged.AddUObject(this, &UPCGameStateWidget::OnRoundsLayoutChanged_Handler);

	CachedStageIdx = PCGameState->GetStageIndex();
	CachedChipCount = PCGameState->GetNumRoundsInStage(CachedStageIdx);
	
	RebuildRoundChipsForStage(CachedStageIdx);
	UpdateRoundChipsState();

	GetWorld()->GetTimerManager().SetTimer(TickHandle, this, &UPCGameStateWidget::TickUpdate, 0.1f, true);

	
}

void UPCGameStateWidget::ReFreshStatic()
{
	if (!PCGameState.IsValid())
		return;

	// 캐러셀 단계면 레이아웃 그대로 두고 타이머/ 아이콘 상태만 갱신
	if (PCGameState->GetCurrentStageType() == EPCStageType::Carousel)
	{
		UpdateRoundChipsState();
		if (Txt_Stage)
		{
			Txt_Stage->SetText(FText::FromString(PCGameState->GetStageLabelString()));
		}
		return;
	}

	const int32 NewStageIdx = PCGameState->GetStageIndex();
	const int32 NewChipCount = PCGameState->GetNumRoundsInStage(NewStageIdx);

	const bool bStageChanged = (NewStageIdx != CachedStageIdx);
	const bool bCountChanged = (NewChipCount != CachedChipCount);

	if (bStageChanged || bCountChanged)
	{
		CachedStageIdx = NewStageIdx;
		CachedChipCount = NewChipCount;

		RebuildRoundChipsForStage(NewStageIdx);
	}

	UpdateRoundChipsState();
	
	if (Txt_Stage)
	{
		Txt_Stage->SetText((FText::FromString(PCGameState->GetStageLabelString())));
	}

	
}

void UPCGameStateWidget::TickUpdate()
{
	if (!PCGameState.IsValid())
		return;
	const float Remain = PCGameState->GetStageRemainingSeconds();
	const float TProg = PCGameState->GetStageProgress();

	if (Txt_Timer)
	{
		const int32 Sec = FMath::RoundToInt(Remain);
		Txt_Timer->SetText(FText::AsNumber(Sec));
	}

	if (Time_Bar)
	{
		Time_Bar->SetPercent(1.f - TProg);
	}

	UpdateRoundChipsState();
}

void UPCGameStateWidget::OnRoundsLayoutChanged_Handler()
{
	if (!PCGameState.IsValid()) return;

	if (PCGameState->GetCurrentStageType() == EPCStageType::Carousel)
	{
		UpdateRoundChipsState();
		return;
	}

	const int32 StageIdx = PCGameState->GetStageIndex();
	CachedStageIdx = StageIdx;
	CachedChipCount = PCGameState->GetNumRoundsInStage(StageIdx);

	RebuildRoundChipsForStage(StageIdx);
	UpdateRoundChipsState();
}

void UPCGameStateWidget::ClearHB()
{
	if (!HB_Rounds) return;
	HB_Rounds->ClearChildren();
	Chips.Reset();
	
}

void UPCGameStateWidget::RebuildRoundChipsForStage(int32 StageIdx)
{
	if (!HB_Rounds || !PCGameState.IsValid()) return;

	ClearHB();

	const int32 Count = PCGameState->GetNumRoundsInStage(StageIdx);
	if (Count <= 0) return;

	Chips.SetNum(Count);

	if (StageIdx == 1)
	{
		SB_StateBar->SetWidthOverride(450.f);
		SB_StateBar->SetHeightOverride(48.f);
		SB_Timer->SetWidthOverride(350.f);
		SB_Timer->SetHeightOverride(10.f);
	}
	else
	{
		SB_StateBar->SetWidthOverride(600.f);
		SB_StateBar->SetHeightOverride(48.f);
		SB_Timer->SetWidthOverride(450.f);
		SB_Timer->SetHeightOverride(10.f);
	}

	for (int32 i = 0; i < Count; ++i)
	{
		UOverlay* Cell = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass());
		UImage* Icon = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
		UImage* Arrow = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());

		Icon->SetDesiredSizeOverride(RoundIconSize);
		Arrow->SetDesiredSizeOverride(ArrowSize);
		
		{
			UOverlaySlot* ArrowSlot = Cell->AddChildToOverlay(Arrow);
			ArrowSlot->SetHorizontalAlignment(HAlign_Center);
			ArrowSlot->SetVerticalAlignment(VAlign_Center);
		}

		{
			UOverlaySlot* IconSlot = Cell->AddChildToOverlay(Icon);
			IconSlot->SetHorizontalAlignment(HAlign_Center);
			IconSlot->SetVerticalAlignment(VAlign_Top);
		}


		UHorizontalBoxSlot* CellSlot = HB_Rounds->AddChildToHorizontalBox(Cell);
		CellSlot->SetPadding(FMargin(5.f,0.f));
		CellSlot->SetHorizontalAlignment(HAlign_Center);
		CellSlot->SetVerticalAlignment(VAlign_Center);
		

		Chips[i].Icon = Icon;
		Chips[i].Arrow = Arrow;
	}

	UpdateRoundChipsState();
		
}


void UPCGameStateWidget::UpdateRoundChipsState()
{
	if (!PCGameState.IsValid() || Chips.Num() <= 0)
		return;

	const int32 S   = PCGameState->GetStageIndex();
	const int32 Cur = PCGameState->GetRoundIndex();

	for (int32 i = 0; i < Chips.Num(); ++i)
	{
		int32 Idx = i + 1;
		const bool bCurrent  = (Idx == Cur);
		const bool bPast     = (Idx <  Cur);
		const bool bPastWin  = bPast ? PCGameState->WasRoundVictory(S, Idx) : false;
		const bool bPastLose = bPast ? PCGameState->WasRoundDefeat(S,Idx) : false;

		const FGameplayTag Major = PCGameState->GetMajorStageForRound(S, i);

		// 아이콘
		if (Chips[i].Icon)
		{
			if (UTexture2D* Tex = PickIconFor(Major, bCurrent, bPastWin, bPastLose))
			{
				Chips[i].Icon->SetBrushFromTexture(Tex, true);
				Chips[i].Icon->SetDesiredSizeOverride(RoundIconSize);
			}
		}

		// 화살표 on/off
		if (Chips[i].Arrow)
		{
			Chips[i].Arrow->SetBrushFromAtlasInterface(Img_Arrow, true);
			Chips[i].Arrow->SetOpacity(bCurrent ? 1.f : 0.f);
			Chips[i].Arrow->SetDesiredSizeOverride(ArrowSize);
		}
	}
}

const FStageIconVariant* UPCGameStateWidget::ChooseVariantFor(FGameplayTag Major) const
{
	if (!IconData) return nullptr;

	if (Major.MatchesTagExact(GameRoundTags::GameRound_PvP))
	{
		return &IconData->PvP;
	}
	else if (Major.MatchesTagExact(GameRoundTags::GameRound_PvE))
	{
		return &IconData->PvE;
	}
	else if (Major.MatchesTagExact(GameRoundTags::GameRound_Carousel))
	{
		return &IconData->Carousel;
	}
	else if (Major.MatchesTagExact(GameRoundTags::GameRound_Start))
	{
		return &IconData->Start;
	}
	return nullptr;
}

UTexture2D* UPCGameStateWidget::PickIconFor(FGameplayTag Major, bool bCurrent, bool bPastWin, bool bPastLose) const
{
	const FStageIconVariant* Variant = ChooseVariantFor(Major);
	if (!Variant) return nullptr;

	if (bCurrent && Variant->Current)
		return Variant->Current;
	if (bPastWin && Variant->Victory)
		return Variant->Victory;
	if (bPastLose && Variant->Defeat)
		return Variant->Defeat;
	
	return Variant->UpComing ? Variant->UpComing : Variant->Current;
}


