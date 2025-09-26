// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ConnectWidget.generated.h"

class UEditableTextBox;
class UButton;
/**
 * 
 */
UCLASS()
class PROJECTPC_API UConnectWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	virtual void NativeConstruct() override;


	UFUNCTION()
	void Connect();

	UPROPERTY(meta = (BindWidget))
	UEditableTextBox* IPAddress;
	
	UPROPERTY(Meta = (BindWidget))
	UButton* ConnectButton;

	
	
};
