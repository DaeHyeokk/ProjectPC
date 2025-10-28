// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RegisterWidget.generated.h"

class UButton;
class UEditableTextBox;
/**
 * 
 */


UCLASS()
class PROJECTPC_API URegisterWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:

	virtual void NativeConstruct() override;


protected:
	
	UPROPERTY(meta = (BindWidget))
	UEditableTextBox* EB_DisplayName = nullptr;

	UPROPERTY(meta = (BindWidget))
	UButton* Btn_Submit;

	UPROPERTY(meta = (BindWidget))
	UButton* Btn_Cancel;
		
	UFUNCTION()
	void OnClicked_Submit();

	UFUNCTION()
	void OnClicked_Cancel();

private:

	bool ValidateID(const FString& ID, FString& OutErr) const;
	
	
};
