// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PlayerMainWidget/PCStepNoticeWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"


void UPCStepNoticeWidget::SetWidget(bool bIsSetUp, const FText& InDetail)
{
	if (!Img_Stage) return;

	if (bIsSetUp)
	{
		Img_Stage->SetBrush(SetUpBrush);
	}
	else
	{
		Img_Stage->SetBrush(BattleBrush);
	}
	
	if (!Txt_Stage) return;
	
	Txt_Stage->SetText(InDetail);

	if (!WidgetAnim) return;
	PlayAnimationForward(WidgetAnim);
}

void UPCStepNoticeWidget::NativeConstruct()
{
	Super::NativeConstruct();

	SetVisibility(ESlateVisibility::Hidden);
}



