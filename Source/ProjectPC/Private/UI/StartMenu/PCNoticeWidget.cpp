// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/StartMenu/PCNoticeWidget.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"




void UPCNoticeWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Btn_OK)
	{
		Btn_OK->OnClicked.AddDynamic(this, &UPCNoticeWidget::ButtonClick);
	}
}


void UPCNoticeWidget::SetMessage(const FText& Message)
{
	if (Tb_Message)
	{
		Tb_Message->SetText(Message);
	}
}

void UPCNoticeWidget::ButtonClick()
{
	SetVisibility(ESlateVisibility::Hidden);
}
