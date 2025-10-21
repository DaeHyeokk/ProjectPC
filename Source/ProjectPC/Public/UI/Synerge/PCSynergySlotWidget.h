// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PCSynergySlotWidget.generated.h"

class UPCSynergyInfoData;
class UBorder;
class ISlateTextureAtlasInterface;
class UTextBlock;
class UImage;
struct FSynergyData;
/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCSynergySlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION()
	void SetData(const FSynergyData& InData);

protected:

	UPROPERTY(meta = (BindWidget))
	UBorder* TierIcon;
	
	UPROPERTY(meta = (BindWidget))
	UImage* SynergyIcon;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* SynergyName;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* SynergyCount;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TierThresholds1;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TierThresholds2;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TierThresholds3;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TierThresholds4;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TierSlash1;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TierSlash2;

	UPROPERTY(EditAnywhere, Category = "Synergy|UI")
	TObjectPtr<UPCSynergyInfoData> SynergyUIData;
	
	
};
