// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Board/PCBoardWidget.h"

#include "Components/TextBlock.h"


void UPCBoardWidget::SetValues(int32 InCur, int32 InMax)
{
	if (TextCur)
		TextCur->SetText(FText::AsNumber(InCur));
	if (TextMax)
		TextMax->SetText(FText::AsNumber(InMax));
}
