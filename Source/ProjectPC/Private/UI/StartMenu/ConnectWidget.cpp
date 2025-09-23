// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/StartMenu/ConnectWidget.h"

#include "Components/Button.h"
#include "Components/EditableTextBox.h"


void UConnectWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ConnectButton)
		ConnectButton->OnClicked.AddDynamic(this, &UConnectWidget::Connect);
}

void UConnectWidget::Connect()
{
	if (!IPAddress) return;

	FString Address = IPAddress->GetText().ToString().TrimStartAndEnd();

	// 포트 미기입 시 기본 7777
	if (!Address.Contains(TEXT(":")))
	{
		Address += TEXT(":7777");
	}

	// “IP:Port” 형식이면 콜론 그대로 사용 가능
	// ClientTravel이 가장 안전
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		// 절대 URL로 접속
		PC->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
	}
}
