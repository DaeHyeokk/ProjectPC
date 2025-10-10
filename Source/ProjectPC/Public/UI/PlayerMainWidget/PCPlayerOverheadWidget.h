// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PCPlayerOverheadWidget.generated.h"

struct FOnAttributeChangeData;

class APCPlayerState;
class UProgressBar;
class UTextBlock;

/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCPlayerOverheadWidget : public UUserWidget
{
	GENERATED_BODY()

private:
	UPROPERTY()
	APCPlayerState* CachedPlayerState;

	const TArray<FLinearColor> PlayerColors = {
		FLinearColor(0.0f, 0.5f, 1.0f),	// 파랑
		FLinearColor(0.3f, 1.0f, 0.3f),  // 연두
		FLinearColor(0.7f, 0.3f, 1.0f),  // 보라
		FLinearColor(1.0f, 0.5f, 0.0f),  // 주황
		FLinearColor(1.0f, 1.0f, 0.0f),  // 노랑
		FLinearColor(0.0f, 1.0f, 1.0f),  // 시안
		FLinearColor(1.0f, 0.0f, 1.0f),  // 마젠타
		FLinearColor(0.7f, 0.0f, 0.0f)   // 빨강
	};
	
protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* PlayerName;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* PlayerLevel;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UProgressBar* HPBar;

public:
	void BindToPlayerState(APCPlayerState* NewPlayerState);
	void SetupPlayerInfo();
	
private:
	void OnPlayerLevelChanged(const FOnAttributeChangeData& Data);
	void OnPlayerHPChanged(const FOnAttributeChangeData& Data);
};
