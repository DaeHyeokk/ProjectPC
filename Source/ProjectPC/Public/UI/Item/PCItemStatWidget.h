// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Blueprint/UserWidget.h"
#include "PCItemStatWidget.generated.h"

class UTextBlock;
class UImage;

/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCItemStatWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void Setup(FGameplayTag ItemStatTag);
	
protected:
	UPROPERTY(EditDefaultsOnly, Category = "StatIcon")
	TMap<FGameplayTag, TSoftObjectPtr<UTexture2D>> StatIcon;
	
	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	UImage* Img_Stat;

	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	UTextBlock* Text_StatValue;
};
