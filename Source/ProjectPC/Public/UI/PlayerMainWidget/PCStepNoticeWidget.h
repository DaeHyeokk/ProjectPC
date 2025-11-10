// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PCStepNoticeWidget.generated.h"

class UImage;
class UTextBlock;
/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCStepNoticeWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void SetWidget(bool bIsSetUp, const FText& InDetail);

private:
	virtual void NativeConstruct() override;

protected:

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Txt_Stage = nullptr;

	UPROPERTY(meta = (BindWidget))
	UImage* Img_Stage = nullptr;

	UPROPERTY(Transient, meta=(BindWidgetAnimOptional))
	UWidgetAnimation* WidgetAnim = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSlateBrush SetUpBrush;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSlateBrush BattleBrush;

	
		
	
	
	
};
