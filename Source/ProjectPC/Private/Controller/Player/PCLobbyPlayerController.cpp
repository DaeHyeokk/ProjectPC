// Fill out your copyright notice in the Description page of Project Settings.


#include "Controller/Player/PCLobbyPlayerController.h"

#include "GameFramework/PlayerState.h"
#include "GameFramework/GameMode/PCLobbyGameMode.h"
#include "GameFramework/GameState/PCLobbyGameState.h"
#include "GameFramework/PlayerState/PCPlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "UI/Lobby/LobbyMenuWidget.h"
#include "UI/StartMenu/StartMenuWidget.h"


void APCLobbyPlayerController::ServerSubmitIdentity_Implementation(const FString& DisplayName)
{
	if (APCPlayerState* PS = GetPlayerState<APCPlayerState>())
	{
		PS->bIdentified = true;
		PS->LocalUserId = DisplayName;
		// 중복 닉 검사
		bool bTaken = false;
		if (const AGameStateBase* GS = GetWorld()->GetGameState())
		{
			for (APlayerState* Other : GS->PlayerArray)
			{
				if (APCPlayerState* PCPlayerState = Cast<APCPlayerState>(Other))
				{
					if (PCPlayerState && PCPlayerState != PS && PCPlayerState->LocalUserId.Equals(DisplayName, ESearchCase::IgnoreCase))
					{
						bTaken = true; break;
					}
				}
			}
		}
		if (bTaken)
		{
			PS->LocalUserId = "";
			ClientRejectIdentity(TEXT("ID already taken."));
		}
	}
}

void APCLobbyPlayerController::ServerSetReady_Implementation(bool bNewReady)
{
	if (auto* PS = GetPlayerState<APCPlayerState>())
	{
		PS->bIsReady = bNewReady;
		if (auto* GS = GetWorld()->GetGameState<APCLobbyGameState>())
			GS->RecountReady();
	}
}

void APCLobbyPlayerController::ServerRequestStart_Implementation()
{
	APCLobbyGameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<APCLobbyGameMode>() : nullptr;
	if (!GM)
	{
		ClientMessage(TEXT("Start denied : GameMode not found"));
		return;
	}

	APCPlayerState* PS = GetPlayerState<APCPlayerState>();
	if (!PS || !PS->bIsLeader)
	{
		ClientMessage(TEXT("Start denied : only the leader can start."));
		return;
	}

	GM->ForceStartByLeader();
}


void APCLobbyPlayerController::BeginPlay()
{
	Super::BeginPlay();
	RefreshUIForMap();
	SetShowMouseCursor(true);
	
}

void APCLobbyPlayerController::BeginPlayingState()
{
	Super::BeginPlayingState();
	RefreshUIForMap();

	if (bPendingLobbyUI && IsOnStartLobbyMap() && IsConnectedToServer())
	{
		ShowLobbyMenuWidget();
		bPendingLobbyUI = false;
	}
}

void APCLobbyPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	if (StartMenuWidget)
	{
		StartMenuWidget->RemoveFromParent();
		StartMenuWidget = nullptr;
	}
	if (LobbyMenuWidget)
	{
		LobbyMenuWidget->RemoveFromParent();
		LobbyMenuWidget = nullptr;
	}
}

void APCLobbyPlayerController::RequestConnectToServer()
{
	HideStartWidget();
	bPendingLobbyUI = true;
	SetLobbyUI();
}

void APCLobbyPlayerController::RefreshUIForMap()
{
	if (!IsLocalController()) return;

	if (IsConnectedToServer())
	{
		ShowStartWidget();
		HideLobbyMenuWidget();
	}	
}

void APCLobbyPlayerController::SetLobbyUI()
{
	if (!IsLocalController()) return;
	
	if (IsConnectedToServer())
	{
		HideStartWidget();
		ShowLobbyMenuWidget();
	}
}

void APCLobbyPlayerController::ClientRejectIdentity_Implementation(const FString& Reason)
{
	ClientMessage(Reason);
}

void APCLobbyPlayerController::ServerNotifyLobbyUIShown_Implementation()
{
	if (APCLobbyGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<APCLobbyGameMode>() : nullptr)
	{
		GameMode->HandlePlayerEnteredLobby(this);
	}
}

void APCLobbyPlayerController::ApplyUIOnly(UUserWidget* FocusWidget)
{
	if (!IsLocalController()) return;

	if (FocusWidget)
		FocusWidget->SetIsFocusable(true);
	FInputModeUIOnly ModeUIOnly;
	ModeUIOnly.SetWidgetToFocus(FocusWidget ? FocusWidget->TakeWidget() : TSharedPtr<SWidget>());
	ModeUIOnly.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(ModeUIOnly);

	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
	
}

void APCLobbyPlayerController::ShowStartWidget()
{
	if (!StartMenuWidgetClass) return;
	if (!StartMenuWidget)
	{
		StartMenuWidget = CreateWidget<UStartMenuWidget>(this, StartMenuWidgetClass);
		if (!StartMenuWidget) return;
	}
	if (!StartMenuWidget->IsInViewport())
		StartMenuWidget->AddToViewport();
	StartMenuWidget->SetVisibility(ESlateVisibility::Visible);
	ApplyUIOnly(StartMenuWidget);
}

void APCLobbyPlayerController::HideStartWidget()
{
	if (StartMenuWidget)
		StartMenuWidget->SetVisibility(ESlateVisibility::Hidden);
}

void APCLobbyPlayerController::ShowLobbyMenuWidget()
{
	if (!LobbyMenuWidget && LobbyMenuWidgetClass)
	{
		LobbyMenuWidget = CreateWidget<ULobbyMenuWidget>(this, LobbyMenuWidgetClass);
		if (!LobbyMenuWidget) return;
	}
	if (!LobbyMenuWidget->IsInViewport())
		LobbyMenuWidget->AddToViewport();
	LobbyMenuWidget->SetVisibility(ESlateVisibility::Visible);
	ApplyUIOnly(LobbyMenuWidget);

	if (IsLocalController() && IsConnectedToServer())
		ServerNotifyLobbyUIShown();
}

void APCLobbyPlayerController::HideLobbyMenuWidget()
{
	if (LobbyMenuWidget)
		LobbyMenuWidget->SetVisibility(ESlateVisibility::Hidden);
}

bool APCLobbyPlayerController::IsOnStartLobbyMap() const
{
	const FString Full = UGameplayStatics::GetCurrentLevelName(GetWorld(), true);
	const FName ShortName = FName(*FPackageName::GetShortName(Full));
	return ShortName == StartLobbyMapName;
}

bool APCLobbyPlayerController::IsConnectedToServer() const
{
	const ENetMode NetMode = GetNetMode();
	return (NetMode == NM_Client || NetMode == NM_ListenServer);
}
