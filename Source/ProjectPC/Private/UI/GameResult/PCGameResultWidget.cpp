// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/GameResult/PCGameResultWidget.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Kismet/KismetSystemLibrary.h"


bool UPCGameResultWidget::Initialize()
{
	if (!Super::Initialize()) return false;

	if (!Btn_Exit) return false;
	Btn_Exit->OnClicked.AddDynamic(this, &UPCGameResultWidget::OnClickedExit);
	if (!Btn_Spectate) return false;
	Btn_Spectate->OnClicked.AddDynamic(this, &UPCGameResultWidget::CloseMenu);

	return true;
}

void UPCGameResultWidget::SetRanking(int32 NewRanking)
{
	if (!Ranking) return;

	auto RankingText = FString::Printf(TEXT("%d위"), NewRanking);
	Ranking->SetText(FText::FromString(RankingText));
}

void UPCGameResultWidget::OpenMenu()
{
	AddToViewport(99999);
}

void UPCGameResultWidget::CloseMenu()
{
	RemoveFromParent();
}

void UPCGameResultWidget::OnClickedExit()
{
	if (UWorld* World = GetWorld())
	{
		if (APlayerController* PC = GetOwningPlayer())
		{
			UKismetSystemLibrary::QuitGame(World, PC, EQuitPreference::Quit, true);
		}
	}
}
