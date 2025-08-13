// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PCShopWidget.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCShopWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPCShopWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UnitSlotWidgetClass")
	TSubclassOf<class UUserWidget> UnitSlotWidgetClass;

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UHorizontalBox* ShopBox;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UButton* Btn_BuyXP;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UButton* Btn_Reroll;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UTextBlock* GoldBalance;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UWidgetSwitcher* WidgetSwitcher;
};
