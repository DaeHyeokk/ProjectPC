// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PCLobbyPlayerController.generated.h"

class UPCNoticeWidget;
class UStartMenuWidget;
class ULobbyMenuWidget;
/**
 * 
 */
UCLASS()
class PROJECTPC_API APCLobbyPlayerController : public APlayerController
{
	GENERATED_BODY()

private:

	virtual void BeginPlay() override;
	virtual void BeginPlayingState() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void PreClientTravel(const FString& PendingURL, ETravelType TravelType, bool bIsSeamlessTravel) override;

public:

	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UStartMenuWidget> StartMenuWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<ULobbyMenuWidget> LobbyMenuWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UUserWidget> BlackWidgetClass;

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

	UFUNCTION(Server, Reliable)
	void ServerSetIdentity();

	// 결과 통지용
	UFUNCTION(Client,Reliable)
	void ClientAcceptedIdentity();

	UFUNCTION(Client, Reliable)
	void ClientRejectIdentity(const FString& Reason);

	// Notice 위젯
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UPCNoticeWidget> NoticeWidgetClass;

	UPROPERTY()
	UPCNoticeWidget* NoticeWidget = nullptr;

	void ShowNotice(const FText& Message);
	void HideNotice();

	UFUNCTION(Server, Reliable)
	void ServerSetReady(bool bNewReady);

	UFUNCTION(Server, Reliable)
	void ServerRequestStart();

	UFUNCTION(Server, Reliable)
	void ServerNotifyLobbyUIShown();

	void ApplyUIOnly(UUserWidget* FocusWidget);

	UFUNCTION(Client, Reliable)
	void ShowFadeWidget();

	void PlayStartBGM();
	void PlayLobbyBGM();
	

private:
	UPROPERTY()
	UStartMenuWidget* StartMenuWidget = nullptr;

	UPROPERTY()
	ULobbyMenuWidget* LobbyMenuWidget = nullptr;

	UPROPERTY()
	UUserWidget* BlackWidget = nullptr;

	void ShowStartWidget();
	void HideStartWidget();
	void ShowLobbyMenuWidget();
	void HideLobbyMenuWidget();

	
	bool IsOnStartLobbyMap() const;
	bool IsConnectedToServer() const;

	bool bPendingLobbyUI = false;

	// sound

	UPROPERTY()
	UAudioComponent* AudioComponent = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	USoundWave* StartBGM;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	USoundWave* LobbyBGM;

	
};
