// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PCPlayerMainWidget.generated.h"

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
	void InitAndBind();

	UFUNCTION(BlueprintCallable)
	void HideWidget();

	UFUNCTION(BlueprintCallable)
	void ShowWidget();

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<class UPCGameStateWidget> W_GameStateWidget;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<class UPCShopWidget> W_ShopWidget;
};
