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

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRegisterCompleted);

UCLASS()
class PROJECTPC_API URegisterWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:

	virtual void NativeConstruct() override;

	
	UPROPERTY(meta = (BindWidget))
	UEditableTextBox* EB_DisplayName = nullptr;

	UPROPERTY(meta = (BindWidget))
	UButton* Btn_Submit;

	UPROPERTY(BlueprintAssignable)
	FOnRegisterCompleted OnRegistered;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Register")
	bool bSubmitToServerOnClose = false;

	UFUNCTION(BLueprintCallable)
	void SetSubmitToServerOnClose(bool bEnable) { bSubmitToServerOnClose = bEnable; }
	
	UFUNCTION()
	void OnClicked_Submit();
	
	
};
