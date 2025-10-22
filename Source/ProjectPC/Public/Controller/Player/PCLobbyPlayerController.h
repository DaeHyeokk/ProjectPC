// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PCLobbyPlayerController.generated.h"

class UStartMenuWidget;
class ULobbyMenuWidget;
/**
 * 
 */
UCLASS()
class PROJECTPC_API APCLobbyPlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	virtual void BeginPlay() override;
	virtual void BeginPlayingState() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UStartMenuWidget> StartMenuWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<ULobbyMenuWidget> LobbyMenuWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	FName StartLobbyMapName;

	UFUNCTION(BlueprintCallable, Category = "Net")
	void RequestConnectToServer();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void RefreshUIForMap();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void SetLobbyUI();
	
	UFUNCTION(Server, Reliable)
	void ServerSubmitIdentity(const FString& DisplayName);

	UFUNCTION(Client, Reliable)
	void ClientRejectIdentity(const FString& Reason);

	UFUNCTION(Server, Reliable)
	void ServerSetReady(bool bNewReady);

	UFUNCTION(Server, Reliable)
	void ServerRequestStart();

	UFUNCTION(Server, Reliable)
	void ServerNotifyLobbyUIShown();

	void ApplyUIOnly(UUserWidget* FocusWidget);


private:
	UPROPERTY()
	UStartMenuWidget* StartMenuWidget = nullptr;

	UPROPERTY()
	ULobbyMenuWidget* LobbyMenuWidget = nullptr;

	void ShowStartWidget();
	void HideStartWidget();
	void ShowLobbyMenuWidget();
	void HideLobbyMenuWidget();

	bool IsOnStartLobbyMap() const;
	bool IsConnectedToServer() const;

	bool bPendingLobbyUI = false;
};
