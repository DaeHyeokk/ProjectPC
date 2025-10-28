// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/StartMenu/StartMenuWidget.h"
#include "Components/TextBlock.h"
#include "Components/EditableTextBox.h"
#include "Components/Button.h"
#include "Controller/Player/PCLobbyPlayerController.h"
#include "GameFramework/GameInstanceSubsystem/ProfileSubsystem.h"
#include "GameFramework/PlayerState/PCPlayerState.h"
#include "UI/StartMenu/PCNoticeWidget.h"
#include "UI/StartMenu/RegisterWidget.h"


void UStartMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (Btn_Register)
		Btn_Register->OnClicked.AddDynamic(this, &UStartMenuWidget::OnClicked_Register);
	if (Btn_JoinLobby)
		Btn_JoinLobby->OnClicked.AddDynamic(this, &UStartMenuWidget::OnClicked_JoinLobby);
	
	// GetWorld()->GetTimerManager().SetTimer(UIRefreshTimer, this, &UStartMenuWidget::RefreshButtons, 0.3f, true, 0.0f);
	// RefreshButtons();
	
}

void UStartMenuWidget::NativeDestruct()
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(UIRefreshTimer);
	}
	Super::NativeDestruct();
}

void UStartMenuWidget::OnClicked_Register()
{
	OpenRegister(true);
}

void UStartMenuWidget::OnClicked_JoinLobby()
{
	UProfileSubsystem* Profile = GetGameInstance()->GetSubsystem<UProfileSubsystem>();
	if (!Profile) return;

	if (!Profile->HasDisplayName())
	{
		ShowNotice(FText::FromString(TEXT("아이디를 등록해주세요.")));
		OpenRegister(true);
		return;
	}
	
	FString CandidateID;
	if (EB_DisplayName)
		CandidateID = EB_DisplayName->GetText().ToString().TrimStartAndEnd();
	
	if (CandidateID.IsEmpty())
	{
		ShowNotice(FText::FromString(TEXT("아이디를 입력해주세요.")));
		if (EB_DisplayName)
			EB_DisplayName->SetKeyboardFocus();
		return;
	}
	
	if (APCLobbyPlayerController* LobbyPlayerController = GetOwningPlayer<APCLobbyPlayerController>())
	{
		LobbyPlayerController->RequestConnectToServer();
		LobbyPlayerController->ServerSetIdentity();
	}
	
}

void UStartMenuWidget::RefreshButtons()
{
	bool bHasName = false;
	if (UProfileSubsystem* Profile = GetGameInstance()->GetSubsystem<UProfileSubsystem>())
		bHasName = Profile->HasDisplayName();

	if (Btn_JoinLobby)
		Btn_JoinLobby->SetIsEnabled(bHasName);

	if (Tb_Status)
	{
		Tb_Status->SetText(FText::FromString(bHasName ? TEXT("Go to Lobby") : TEXT("You Don't Have ID, You do Register ID")));
	}
}

void UStartMenuWidget::OpenRegister(bool bFocusName)
{
	if (!RegisterWidgetClass) return;

	if (APlayerController* PlayerController = GetOwningPlayer())
	{
		if (!RegisterWidget)
		{
			RegisterWidget = CreateWidget<URegisterWidget>(PlayerController, RegisterWidgetClass);
			if (RegisterWidget)
			{
				RegisterWidget->AddToViewport(100);
			}
		}

		if (RegisterWidget)
		{
			
			RegisterWidget->SetVisibility(ESlateVisibility::Visible);
			RegisterWidget->SetIsFocusable(true);

			FInputModeUIOnly Mode;
			Mode.SetWidgetToFocus(RegisterWidget->TakeWidget());
			Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			PlayerController->SetInputMode(Mode);
			PlayerController->bShowMouseCursor = true;
			
			if (bFocusName)
			RegisterWidget->SetKeyboardFocus();
		}
			
	}
}


void UStartMenuWidget::ConnectToServer()
{
	if (APlayerController* PlayerController = GetOwningPlayer())
	{
		if (APCLobbyPlayerController* LobbyPlayerController = Cast<APCLobbyPlayerController>(PlayerController))
		{
			LobbyPlayerController->RequestConnectToServer();
		}
		SetVisibility(ESlateVisibility::Hidden);
	}
}

void UStartMenuWidget::ShowNotice(const FText& Message)
{
	if (APlayerController* InPlayerController = GetOwningPlayer())
	{
		if (APCLobbyPlayerController* LobbyPlayerController = Cast<APCLobbyPlayerController>(InPlayerController))
		{
			LobbyPlayerController->ShowNotice(Message);
		}
	}
}


