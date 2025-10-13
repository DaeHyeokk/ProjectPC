// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PlayerMainWidget/PCRoundCellWidget.h"

#include "Components/Image.h"


void UPCRoundCellWidget::SetArrowOn(bool bOn)
{
	if (ArrowIcon)
	{
		ArrowIcon->SetRenderOpacity(bOn ? 1.f : 0.f);
	}
}
