// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PCShopWidget.generated.h"

struct FOnAttributeChangeData;

class APCCombatPlayerController;
class APCPlayerState;
class UButton;
class UHorizontalBox;
class UImage;
class UOverlay;
class UPCUnitSlotWidget;
class UProgressBar;
class UTextBlock;
class UWidgetSwitcher;

/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCShopWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual bool Initialize() override;
	virtual void NativeDestruct() override;

private:
	UPROPERTY()
	TArray<UPCUnitSlotWidget*> UnitSlotWidgets;
	
	UPROPERTY()
	APCPlayerState* CachedPlayerState;

	UPROPERTY()
	TWeakObjectPtr<APCCombatPlayerController> CachedController;
	
public:
	void BindToPlayerState(APCPlayerState* NewPlayerState);

	void InitWithPC(APCCombatPlayerController* InPC);

protected:
	int32 PlayerLevel;
	int32 PlayerMaxXP;
	int32 PlayerXP;
	int32 PlayerGold;

	UPROPERTY(EditDefaultsOnly, Category = "UnitSlotWidgetClass")
	TSubclassOf<UUserWidget> UnitSlotWidgetClass;
	
	// UMG Widget
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UHorizontalBox> PlayerShopBox;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UHorizontalBox> ShopBox;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UOverlay> SellBox;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UWidgetSwitcher> WidgetSwitcher;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButton> Btn_BuyXP;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButton> Btn_Reroll;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButton> Btn_ShopLock;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> GoldBalance;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> WinningStreak;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UImage> Img_ShopLock;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UImage> Img_WinningStreak;

	// Cost Probability
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> Cost1;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> Cost2;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> Cost3;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> Cost4;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> Cost5;

	// Level
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> Level;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> XP;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UProgressBar> XPBar;

	// ShopLock
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TObjectPtr<UTexture2D> ShopLock;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TObjectPtr<UTexture2D> ShopUnlock;

	// WinningStreak
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TObjectPtr<UTexture2D> Winning;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TObjectPtr<UTexture2D> Losing;

public:
	UFUNCTION()
	void SetupShopSlots();
	UFUNCTION()
	void SetupPlayerInfo();
	
private:
	UFUNCTION()
	void OnClickedBuyXP();
	UFUNCTION()
	void OnClickedReroll();
	UFUNCTION()
	void OnClickedShopLock();
	
	void OnPlayerLevelChanged(const FOnAttributeChangeData& Data);
	void OnPlayerXPChanged(const FOnAttributeChangeData& Data);
	void OnPlayerGoldChanged(const FOnAttributeChangeData& Data);
	void OnPlayerWinningStreakChanged(int32 NewWinningStreak);

public:
	void SetSlotHidden(int32 SlotIndex);
	void ShowPlayerShopBox() const;
	void ShowSellBox() const;
	bool IsScreenPointInSellBox(const FVector2D& Point) const;
};
