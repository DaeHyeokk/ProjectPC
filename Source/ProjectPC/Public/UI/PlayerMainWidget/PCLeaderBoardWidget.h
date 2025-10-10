// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PCLeaderBoardWidget.generated.h"

class UVerticalBox;

/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCLeaderBoardWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UVerticalBox* PlayerBox;

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "PlayerRowWidgetClass")
	TSubclassOf<UUserWidget> PlayerRowWidgetClass;
};
