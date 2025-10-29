// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Loading/PCLoadingWidget.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"


void UPCLoadingWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (Progress)
	{
		Progress->SetPercent(0.f);
	}

	if (DetailText)
	{
		DetailText->SetText(FText::FromString(TEXT("Preparing...")));
	}
	
}


void UPCLoadingWidget::SetProgress(float InProgress, const FText& InDetail)
{
	const float P = FMath::Clamp(InProgress, 0.f, 1.f);

	if (Progress)
	{
		Progress->SetPercent(P);
	}
	if (DetailText)
	{
		DetailText->SetText(InDetail);
	}
}

