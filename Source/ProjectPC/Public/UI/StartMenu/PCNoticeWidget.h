// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PCNoticeWidget.generated.h"

class UButton;
class UTextBlock;
/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCNoticeWidget : public UUserWidget
{
	GENERATED_BODY()


public:

	void SetMessage(const FText& Message);

protected:

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Tb_Message;

	UPROPERTY(meta = (BindWidget))
	UButton* Btn_OK;

	UFUNCTION()
	void ButtonClick();

	virtual void NativeConstruct() override;
	
	
};
