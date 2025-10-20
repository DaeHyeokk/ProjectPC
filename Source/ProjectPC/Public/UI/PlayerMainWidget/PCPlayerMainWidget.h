// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PCPlayerMainWidget.generated.h"

class UPCPlayerInventoryWidget;
class UPCShopWidget;
class UPCHeroStatusHoverPanel;
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
	UPCShopWidget* GetShopWidget() { return W_ShopWidget;}

	UFUNCTION(BlueprintCallable)
	UPCPlayerInventoryWidget* GetInventoryWidget() { return W_InventoryWidget;}

	void SetShopWidgetVisible(bool bVisible);
	
protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<class UPCGameStateWidget> W_GameStateWidget;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<class UPCLeaderBoardWidget> W_LeaderBoardWidget;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<class UPCHeroStatusHoverPanel> W_HeroStatus;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<class UPCShopWidget> W_ShopWidget;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<class UPCPlayerInventoryWidget> W_InventoryWidget;
	
};
