// Fill out your copyright notice in the Description page of Project Settings.


#include "Controller/Player/LobbyPlayerController.h"

#include "GameFramework/PlayerState.h"
#include "GameFramework/GameMode/PCLobbyGameMode.h"
#include "GameFramework/GameState/PCLobbyGameState.h"
#include "GameFramework/PlayerState/PCPlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "UI/Lobby/LobbyMenuWidget.h"
#include "UI/StartMenu/StartMenuWidget.h"


void ALobbyPlayerController::ServerSubmitIdentity_Implementation(const FString& DisplayName)
{
	if (APCPlayerState* PS = GetPlayerState<APCPlayerState>())
	{
		PS->bIdentified = true;
		// (선택) 중복 닉 검사
		bool bTaken = false;
		if (const AGameStateBase* GS = GetWorld()->GetGameState())
		{
			for (APlayerState* Other : GS->PlayerArray)
			{
				if (Other && Other != PS &&
					Other->GetPlayerName().Equals(DisplayName, ESearchCase::IgnoreCase))
				{
					bTaken = true; break;
				}
			}
		}
		if (bTaken) { ClientRejectIdentity(TEXT("ID already taken.")); return; }

		PS->LocalUserId = DisplayName;     // ★ 로그인 ID 저장
		PS->SetPlayerName(DisplayName);    // APlayerState의 PlayerName도 동기화
		PS->ForceNetUpdate();
	}
}

void ALobbyPlayerController::ServerSetReady_Implementation(bool bNewReady)
{
	if (auto* PS = GetPlayerState<APCPlayerState>())
	{
		PS->bIsReady = bNewReady;
		if (auto* GS = GetWorld()->GetGameState<APCLobbyGameState>())
			GS->RecountReady();
	}
}

void ALobbyPlayerController::ServerRequestStart_Implementation()
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


void ALobbyPlayerController::BeginPlay()
{
	Super::BeginPlay();
	RefreshUIForMap();
	SetShowMouseCursor(true);
	
}

void ALobbyPlayerController::BeginPlayingState()
{
	Super::BeginPlayingState();
	RefreshUIForMap();

	if (bPendingLobbyUI && IsOnStartLobbyMap() && IsConnectedToServer())
	{
		ShowLobbyMenuWidget();
		bPendingLobbyUI = false;
	}
}

void ALobbyPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
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

void ALobbyPlayerController::RequestConnectToServer(const FString& Address)
{
	HideStartWidget();
	bPendingLobbyUI = true;
	SetLobbyUI();
}

void ALobbyPlayerController::RefreshUIForMap()
{
	if (!IsLocalController()) return;

	if (IsConnectedToServer())
	{
		ShowStartWidget();
		HideLobbyMenuWidget();
	}	
}

void ALobbyPlayerController::SetLobbyUI()
{
	if (!IsLocalController()) return;
	
	if (IsConnectedToServer())
	{
		HideStartWidget();
		ShowLobbyMenuWidget();
	}
}

void ALobbyPlayerController::ClientRejectIdentity_Implementation(const FString& Reason)
{
	ClientMessage(Reason);
}

void ALobbyPlayerController::ServerNotifyLobbyUIShown_Implementation()
{
	if (APCLobbyGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<APCLobbyGameMode>() : nullptr)
	{
		GameMode->HandlePlayerEnteredLobby(this);
	}
}

void ALobbyPlayerController::ApplyUIOnly(UUserWidget* FocusWidget)
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

void ALobbyPlayerController::ShowStartWidget()
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

void ALobbyPlayerController::HideStartWidget()
{
	if (StartMenuWidget)
		StartMenuWidget->SetVisibility(ESlateVisibility::Hidden);
}

void ALobbyPlayerController::ShowLobbyMenuWidget()
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

void ALobbyPlayerController::HideLobbyMenuWidget()
{
	if (LobbyMenuWidget)
		LobbyMenuWidget->SetVisibility(ESlateVisibility::Hidden);
}

bool ALobbyPlayerController::IsOnStartLobbyMap() const
{
	const FString Full = UGameplayStatics::GetCurrentLevelName(GetWorld(), true);
	const FName ShortName = FName(*FPackageName::GetShortName(Full));
	return ShortName == StartLobbyMapName;
}

bool ALobbyPlayerController::IsConnectedToServer() const
{
	const ENetMode NetMode = GetNetMode();
	return (NetMode == NM_Client || NetMode == NM_ListenServer);
}
