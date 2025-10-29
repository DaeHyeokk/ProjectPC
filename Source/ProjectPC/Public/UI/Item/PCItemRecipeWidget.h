// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Blueprint/UserWidget.h"
#include "PCItemRecipeWidget.generated.h"

class UVerticalBox;

/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCItemRecipeWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void Setup(FGameplayTag BaseItemTag);
	
protected:
	UPROPERTY(EditDefaultsOnly, Category = "ItemRecipeRowWidgetClass")
	TSubclassOf<UUserWidget> ItemRecipeRowWidgetClass;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UVerticalBox> ItemInfoBox;

private:
	FGameplayTag CachedBaseItemTag;

	void SetRecipeRow();
};
