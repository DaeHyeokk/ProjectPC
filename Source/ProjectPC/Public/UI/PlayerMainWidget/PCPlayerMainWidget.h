// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PCPlayerMainWidget.generated.h"

class UPCLeaderBoardWidget;
class UPCSynergyPanelWidget;
class UPCSynergyComponent;
class UPCPlayerInventoryWidget;
class UPCShopWidget;
class UPCHeroStatusHoverPanel;
class UPCGameStateWidget;
class APCCombatGameState;
class APCPlayerState;
/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCPlayerMainWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void InitAndBind(APCCombatGameState* PCCombatGameState);

	UFUNCTION(BlueprintCallable)
	UPCHeroStatusHoverPanel* GetHeroStatusWidget() { return W_HeroStatus;}

	UFUNCTION(BlueprintCallable)
	UPCShopWidget* GetShopWidget() const { return W_ShopWidget;}

	UFUNCTION(BlueprintCallable)
	UPCPlayerInventoryWidget* GetInventoryWidget() const { return W_InventoryWidget;}

	UFUNCTION(BlueprintCallable)
	UPCSynergyPanelWidget* GetSynergyWidget() const { return W_SynergyWidget;}

	UFUNCTION(BlueprintCallable)
	UPCLeaderBoardWidget* GetLeaderBoardWidget() const { return W_LeaderBoardWidget;}

	void SetShopWidgetVisible(bool bVisible);
	
protected:
	virtual void NativeConstruct() override;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UPCGameStateWidget> W_GameStateWidget;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UPCLeaderBoardWidget> W_LeaderBoardWidget;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UPCHeroStatusHoverPanel> W_HeroStatus;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UPCShopWidget> W_ShopWidget;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UPCPlayerInventoryWidget> W_InventoryWidget;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UPCSynergyPanelWidget> W_SynergyWidget;
	
};
