// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PCShopWidget.generated.h"

struct FOnAttributeChangeData;

class UHorizontalBox;
class UOverlay;
class UButton;
class UTextBlock;
class UWidgetSwitcher;
class UProgressBar;

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
	TSubclassOf<UUserWidget> UnitSlotWidgetClass;

	void BindToPlayerState(class APCPlayerState* NewPlayerState);

protected:
	int32 PlayerLevel;
	int32 PlayerMaxXP;
	int32 PlayerXP;
	int32 PlayerGold;
	
	// UMG Widget
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UHorizontalBox* PlayerShopBox;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UHorizontalBox* ShopBox;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UOverlay* SellBox;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* Btn_BuyXP;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* Btn_Reroll;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* GoldBalance;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UWidgetSwitcher* WidgetSwitcher;

	// Cost Probability
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* Cost1;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* Cost2;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* Cost3;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* Cost4;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* Cost5;

	// Level
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* Level;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* XP;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UProgressBar* XPBar;

public:
	UFUNCTION(BlueprintCallable)
	void OpenMenu();
	UFUNCTION(BlueprintCallable)
	void CloseMenu();
	UFUNCTION(BlueprintCallable)
	void SetupShopSlots();
	UFUNCTION(BlueprintCallable)
	void SetupPlayerInfo();
	
private:
	UFUNCTION()
	void OnClickedBuyXP();
	UFUNCTION()
	void OnClickedReroll();
	
	void OnPlayerLevelChanged(const FOnAttributeChangeData& Data);
	void OnPlayerXPChanged(const FOnAttributeChangeData& Data);
	void OnPlayerGoldChanged(const FOnAttributeChangeData& Data);

public:
	void SetSlotHidden(int32 SlotIndex);
	void ShowPlayerShopBox() const;
	void ShowSellBox() const;
	bool IsScreenPointInSellBox(const FVector2D& Point) const;
};
