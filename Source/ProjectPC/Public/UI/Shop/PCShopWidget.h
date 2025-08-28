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

protected:
	virtual bool Initialize() override;
	
public:
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

	// Cost Probability
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UTextBlock* Cost1;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UTextBlock* Cost2;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UTextBlock* Cost3;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UTextBlock* Cost4;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UTextBlock* Cost5;

private:
	UFUNCTION()
	void OnClickedBuyXP();
	UFUNCTION()
	void OnClickedReroll();

public:
	UFUNCTION(BlueprintCallable)
	void OpenMenu();
	UFUNCTION(BlueprintCallable)
	void CloseMenu();
	UFUNCTION(BlueprintCallable)
	void SetupShopSlots();

	
};
