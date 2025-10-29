// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "StartMenuWidget.generated.h"

class UPCNoticeWidget;
class URegisterWidget;
class UTextBlock;
class UButton;
class UEditableTextBox;
/**
 * 
 */
UCLASS()
class PROJECTPC_API UStartMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	URegisterWidget* GetRegisterWidget() const { return RegisterWidget; }
	
protected:

	UFUNCTION()
	void OnClicked_JoinLobby();

	UFUNCTION()
	void OnClicked_Register();
	
	UFUNCTION()
	void RefreshButtons();
	
	void OpenRegister(bool bFocusName = true);
	void ConnectToServer();
	void ShowNotice(const FText& Message);
	
	
	UPROPERTY(Meta = (BindWidget))
	UEditableTextBox* EB_DisplayName;

	UPROPERTY(Meta = (BindWidget))
	UButton* Btn_Register;

	UPROPERTY(Meta = (BindWidget))
	UButton* Btn_JoinLobby;

	UPROPERTY(Meta = (BindWidget))
	UTextBlock* Tb_Status;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Register")
	TSubclassOf<URegisterWidget> RegisterWidgetClass;

private:
	UPROPERTY()
	URegisterWidget* RegisterWidget = nullptr;
	
	FTimerHandle UIRefreshTimer;
	
	
	
};
