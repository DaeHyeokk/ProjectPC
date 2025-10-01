// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PCGameResultWidget.generated.h"

class UTextBlock;
class UButton;

/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCGameResultWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual bool Initialize() override;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* Ranking;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* Btn_Exit;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* Btn_Spectate;

public:
	UFUNCTION(BlueprintCallable)
	void SetRanking(int32 NewRanking);

	UFUNCTION(BlueprintCallable)
	void OpenMenu();
	
private:
	UFUNCTION()
	void OnClickedExit();
	UFUNCTION()
	void OnClickedSpectate();
};
