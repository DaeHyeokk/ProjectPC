// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PlayerMainWidget/PCGameStateWidget.h"

#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "GameFramework/GameState/PCCombatGameState.h"
#include "Kismet/GameplayStatics.h"


void UPCGameStateWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	PCGameState = Cast<APCCombatGameState>(UGameplayStatics::GetGameState(this));
	if (PCGameState.IsValid())
	{
		RepHandle = PCGameState->OnStageRuntimeChanged.AddUObject(this, &UPCGameStateWidget::ReFreshStatic);
		ReFreshStatic();
	}

	GetWorld()->GetTimerManager().SetTimer(TickHandle, this, &UPCGameStateWidget::TickUpdate, 0.1f, true);
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

void UPCGameStateWidget::ReFreshStatic()
{
	if (!PCGameState.IsValid())
		return;
	if (Txt_Stage)
	{
		Txt_Stage->SetText((FText::FromString(PCGameState->GetStageLabelString())));
	}
	if (Img_Stage)
	{
		if (UTexture2D** Found = StageIcons.Find(PCGameState->GetCurrentStageType()))
			Img_Stage->SetBrushFromTexture(*Found, true);
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
}

