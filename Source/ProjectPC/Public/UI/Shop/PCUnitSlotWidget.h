// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DataAsset/UI/PCSynergyInfoData.h"
#include "Shop/PCShopUnitData.h"
#include "PCUnitSlotWidget.generated.h"

struct FStreamableHandle;

class UButton;
class UImage;
class UTextBlock;
class UTexture2D;

/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCUnitSlotWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual bool Initialize() override;
	
public:
	void Setup(const FPCShopUnitData& UnitData, bool bIsShopSlot, int32 NewSlotIndex = -1);

protected:
	int32 SlotIndex = -1;
	int32 UnitCost;

	UPROPERTY(EditDefaultsOnly, Category = "SynergyData")
	TObjectPtr<UPCSynergyInfoData> SynergyInfo;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* Btn_UnitSlot;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UImage* Img_CostBorder;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UImage* Img_UnitThumbnail;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UImage* Img_Species;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UImage* Img_Job;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* Text_Cost;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* Text_UnitName;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* Text_Species;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* Text_Job;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	UTexture2D* Cost1Border;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	UTexture2D* Cost2Border;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	UTexture2D* Cost3Border;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	UTexture2D* Cost4Border;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	UTexture2D* Cost5Border;
	
public:
	UFUNCTION()
	void SetupButton();
	
	UFUNCTION()
	void OnClickedUnitSlot();

	void SetSlotHidden(bool IsHidden);

private:
	FString TagToString(FGameplayTag Tag);
};
