// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Lobby/PlayerDataWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "GameFramework/PlayerState/PCPlayerState.h"

void UPlayerDataWidget::SetEmpty()
{
	if (Tb_Name)
		Tb_Name->SetText(FText::FromString(""));
	if (Img_Leader)
		Img_Leader->SetVisibility(ESlateVisibility::Hidden);
	if (Img_Ready)
		Img_Ready->SetVisibility(ESlateVisibility::Hidden);
}

void UPlayerDataWidget::SetPlayer(APCPlayerState* PlayerState)
{
	if (!PlayerState) { SetEmpty(); return; }

	// LocalUserId가 비어있으면 이름을 아예 비우거나 "Guest" 등으로
	const FString NameToShow = PlayerState->LocalUserId; // ← 폴백 제거

	if (Tb_Name)    Tb_Name->SetText(NameToShow.IsEmpty() ? FText::GetEmpty()
														  : FText::FromString(NameToShow));
	if (Img_Leader) Img_Leader->SetVisibility(PlayerState->bIsLeader ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	if (Img_Ready)  Img_Ready->SetVisibility(PlayerState->bIsReady  ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
}
