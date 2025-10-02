// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PCBoardWidget.generated.h"


class UImage;
class UTextBlock;
/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCBoardWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Setter")
	void SetValues(int32 InCur, int32 InMax);

protected:
	
	UPROPERTY(meta = (BindWidget))
	UImage* Icon = nullptr;

	UPROPERTY(Meta = (BindWidget))
	UTextBlock* TextCur = nullptr;

	UPROPERTY(Meta = (BindWidget))
	UTextBlock* TextSlash = nullptr;

	UPROPERTY(Meta = (BindWidget))
	UTextBlock* TextMax = nullptr;
	
	
	
	
	
};
