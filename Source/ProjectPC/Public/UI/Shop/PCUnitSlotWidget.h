// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Shop/PCShopUnitData.h"
#include "PCUnitSlotWidget.generated.h"

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
	void Setup(FPCShopUnitData UnitData);

	UFUNCTION()
	void OnClickedUnitSlot();
	
protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UButton* Btn_UnitSlot;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UImage* Img_CostBorder;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UImage* Img_UnitThumbnail;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UTextBlock* Text_Cost;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UTextBlock* Text_UnitName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	class UTexture2D* Cost1Border;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	class UTexture2D* Cost2Border;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	class UTexture2D* Cost3Border;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	class UTexture2D* Cost4Border;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	class UTexture2D* Cost5Border;
};
