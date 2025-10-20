// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PCRoundCellWidget.generated.h"

class UImage;
/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCRoundCellWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	UImage* StageIcon = nullptr;

	UPROPERTY(meta = (BindWidget))
	UImage* ArrowIcon = nullptr;

	UFUNCTION(BlueprintCallable)
	void SetArrowOn(bool bOn);
	
	
	
	
};
