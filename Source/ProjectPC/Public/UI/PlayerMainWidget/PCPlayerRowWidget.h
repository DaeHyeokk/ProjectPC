// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DataAsset/Player/PCDataAsset_PlayerPortrait.h"
#include "PCPlayerRowWidget.generated.h"

struct FOnAttributeChangeData;

class APCPlayerState;
class UTextBlock;
class UImage;

/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCPlayerRowWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Portrait")
	UPCDataAsset_PlayerPortrait* PlayerPortrait;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* PlayerName;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* PlayerHP;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UImage* CircularHPBar;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UImage* Img_Portrait;

public:
	void SetupPlayerInfo(FString NewPlayerName, float NewPlayerHP, FGameplayTag NewPlayerCharacterTag);

	void UpdatePlayerHP(float NewPlayerHP);
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void SetHP(float HPPercent);
};
