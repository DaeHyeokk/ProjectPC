// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DataAsset/Player/PCDataAsset_PlayerPortrait.h"
#include "PCPlayerRowWidget.generated.h"

struct FOnAttributeChangeData;

class APCPlayerState;
class UButton;
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
	virtual bool Initialize() override;
	
	UPROPERTY(EditDefaultsOnly, Category = "Portrait")
	TObjectPtr<UPCDataAsset_PlayerPortrait> PlayerPortrait;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> PlayerName;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> PlayerHP;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UImage> CircularHPBar;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UImage> Img_Portrait;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButton> Btn_CameraSwitch;

private:
	UPROPERTY()
	APCPlayerState* CachedPlayerState;

protected:
	UPROPERTY(BlueprintReadOnly)
	int32 WinningStreak = 0;

public:
	void SetupPlayerInfo(APCPlayerState* NewPlayerState);

	void UpdatePlayerHP(const FOnAttributeChangeData& Data);
	void SetWinningStreak(int32 NewWinningStreak);
	void ExpandRenderSize();
	void RestoreRenderSize();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void SetHP(float HPPercent);
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void SetWinningFlame();

private:
	UFUNCTION()
	void SwitchCamera();
};
