// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PCLoadingWidget.generated.h"

class UTextBlock;
class UProgressBar;
/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCLoadingWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	void SetProgress(float InProgress, const FText& InDetail);

	UFUNCTION(BlueprintCallable)
	void PlayFadeOut();

protected:

	virtual void NativeOnInitialized() override;
	virtual void OnAnimationFinished_Implementation(const UWidgetAnimation* Animation) override;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* Progress = nullptr;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* DetailText = nullptr;

	UPROPERTY(Transient, meta=(BindWidgetAnimOptional))
	UWidgetAnimation* FadeOut = nullptr;
		
	
};
