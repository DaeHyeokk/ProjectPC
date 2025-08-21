// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/Lobby/PlayerRowData.h"
#include "PlayerDataWidget.generated.h"

/**
 * 
 */

class APCPlayerState;
class UTextBlock;
class UImage;

UCLASS()
class PROJECTPC_API UPlayerDataWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seat")
	int32 SeatIndex = -1;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Tb_Name;

	UPROPERTY(meta = (BindWidget))
	UImage* Img_PlayerIcon;

	UPROPERTY(meta = (BindWidget))
	UImage* Img_Leader;

	UPROPERTY(meta = (BindWidget))
	UImage* Img_Ready;

	UFUNCTION(BlueprintCallable)
	void SetEmpty();

	UFUNCTION(BlueprintCallable)
	void SetPlayer(APCPlayerState* PlayerState);
	
	
	
	
};
