// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
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
	void Setup(FName UnitName);

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
};
