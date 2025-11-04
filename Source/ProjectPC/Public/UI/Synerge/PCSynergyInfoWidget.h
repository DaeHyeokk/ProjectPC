// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PCSynergyInfoWidget.generated.h"

struct FSynergyData;
struct FPCSynergyUIRow;
class UVerticalBox;
class UTextBlock;
class UImage;
/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCSynergyInfoWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	UImage* SynergyIcon = nullptr;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* SynergyName = nullptr;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Summary = nullptr;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* SynergyDescription = nullptr;

	UPROPERTY(meta = (BindWidget))
	UVerticalBox* TierEffectInfo = nullptr;

	UFUNCTION()
	void SetSynergyInfo(const FPCSynergyUIRow& UI, const FSynergyData& Runtime);

private:

	UPROPERTY(EditDefaultsOnly)
	UObject* SynergyInfoFont;
	
	
	
};
