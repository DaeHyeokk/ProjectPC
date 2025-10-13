// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PlayerMainWidget/PCGameStateWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/HorizontalBox.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
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
		PCGameState->OnStageRuntimeChanged.Remove(RepHandle);
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

	// LayOutHandle = PCGameState->OnRoundsLayoutChanged.AddLambda([this]()
	// {
	// 	if (!PCGameState.IsValid())
	// 		return;
	// 	RebuildRoundChipsForStage(PCGameState->GetStageIndex());
	// 	UpdateRoundChipsState();
	// });

	GetWorld()->GetTimerManager().SetTimer(TickHandle, this, &UPCGameStateWidget::TickUpdate, 0.1f, true);

	// RebuildRoundChipsForStage(PCGameState->GetStageIndex());
	// UpdateRoundChipsState();
}

void UPCGameStateWidget::ReFreshStatic()
{
	if (!PCGameState.IsValid())
		return;
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

	//UpdateRoundChipsState();
}

// void UPCGameStateWidget::ClearHB()
// {
// 	if (!HB_Rounds) return;
// 	HB_Rounds->ClearChildren();
// 	Chips.Reset();
// 	
// }
//
// void UPCGameStateWidget::RebuildRoundChipsForStage(int32 StageIdx)
// {
// 	if (!HB_Rounds || !PCGameState.IsValid()) return;
//
// 	ClearHB();
//
// 	const int32 Count = PCGameState->GetNumRoundsInStage(StageIdx);
// 	if (Count <= 0) return;
//
// 	Chips.SetNum(Count);
//
// 	for (int32 i = 0; i < Count; ++i)
// 	{
// 		UHorizontalBox* CellBox = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
// 		UImage* Icon = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
// 		UImage* Arrow = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
//
// 		//UHorizontalBox* Slot
// 	}
// }

// void UPCGameStateWidget::UpdateRoundChipsState()
// {
// }
//
// UTexture2D* UPCGameStateWidget::ResolveIconForRound(int32 StageIdx, int32 RoundIdx, bool bCurrent, bool bPastWin,
// 	bool bPastLose) const
// {
// }
//
//
// void UPCGameStateWidget::UpdateArrow(bool bForce)
// {
// }

