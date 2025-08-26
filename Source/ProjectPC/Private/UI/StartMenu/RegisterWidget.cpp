// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/StartMenu/RegisterWidget.h"

#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Controller/Player/PCLobbyPlayerController.h"
#include "GameFramework/GameInstanceSubsystem/ProfileSubsystem.h"


void URegisterWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (Btn_Submit)
		Btn_Submit->OnClicked.AddDynamic(this, &URegisterWidget::OnClicked_Submit);
}

void URegisterWidget::OnClicked_Submit()
{
	if (!EB_DisplayName) return;

	FString Name = EB_DisplayName->GetText().ToString().TrimStartAndEnd().Left(24);
	if (Name.IsEmpty()) return;
	
	if (UProfileSubsystem* Profile = GetGameInstance()->GetSubsystem<UProfileSubsystem>())
	{
		Profile->SetDisplayName(Name);
	}
	
	if(bSubmitToServerOnClose)
	{
		if (APCLobbyPlayerController* LobbyPlayerController = GetOwningPlayer<APCLobbyPlayerController>())
		{
			const ENetMode NetMode = LobbyPlayerController->GetNetMode();
			if (NetMode == NM_Client || NetMode == NM_ListenServer)
			{
				LobbyPlayerController->ServerSubmitIdentity(Name);
			}
		}
	}
	
	OnRegistered.Broadcast();
	SetVisibility(ESlateVisibility::Hidden);
	
}
