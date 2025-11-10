// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/StartMenu/RegisterWidget.h"

#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Controller/Player/PCLobbyPlayerController.h"
#include "GameFramework/GameInstanceSubsystem/ProfileSubsystem.h"
#include "GameFramework/PlayerState/PCPlayerState.h"


void URegisterWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (Btn_Submit)
		Btn_Submit->OnClicked.AddDynamic(this, &URegisterWidget::OnClicked_Submit);

	if (Btn_Cancel)
		Btn_Cancel->OnClicked.AddDynamic(this, &URegisterWidget::OnClicked_Cancel);
}

void URegisterWidget::OnClicked_Submit()
{
	if (!EB_DisplayName) return;

	FString Name = EB_DisplayName->GetText().ToString().TrimStartAndEnd().Left(24);
	if (Name.IsEmpty()) return;
	
	if (UProfileSubsystem* Profile = GetGameInstance()->GetSubsystem<UProfileSubsystem>())
	{
		Profile->SetUserID(Name);
		
		if (APlayerController* PC = GetOwningPlayer())
		{
			if (APCLobbyPlayerController* LobbyPC = Cast<APCLobbyPlayerController>(PC))
			{
				if (APCPlayerState* PS = LobbyPC->GetPlayerState<APCPlayerState>())
				{
					PS->LocalUserId = Profile->GetUserID();
				}
			}
		}
	}

	if (APCLobbyPlayerController* LobbyPlayerController = GetOwningPlayer<APCLobbyPlayerController>())
	{
		FString Err;
		if (!ValidateID(Name, Err))
		{
			LobbyPlayerController->ShowNotice(FText::FromString(Err));
			if (EB_DisplayName)
				EB_DisplayName->SetKeyboardFocus();
			return;
		}
		
		const ENetMode NetMode = LobbyPlayerController->GetNetMode();
		if (NetMode == NM_Client || NetMode == NM_ListenServer)
		{
			LobbyPlayerController->ServerSubmitIdentity(Name);
		}
	}
}

void URegisterWidget::OnClicked_Cancel()
{
	SetVisibility(ESlateVisibility::Hidden);
}

bool URegisterWidget::ValidateID(const FString& ID, FString& OutErr) const
{
	const FString S = ID.TrimStartAndEnd();
	if (S.Len() < 3 || S.Len() > 16)
	{
		OutErr = TEXT("ID must be 3 ~ 16 Characters.");
		return false;
	}

	for (TCHAR c : S)
	{
		if (!(FChar::IsAlnum(c) || c == TEXT('-')) || c == TEXT('_'))
		{
			OutErr = TEXT("Only Letters, numbers, and Underscore are Allowed.");
			return false;
		}
	}

	return true;
}
