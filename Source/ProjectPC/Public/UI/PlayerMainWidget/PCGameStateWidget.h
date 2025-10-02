// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PCGameStateWidget.generated.h"

class APCCombatGameState;
enum class EPCStageType : uint8;
class UImage;
class UProgressBar;
class UTextBlock;
/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCGameStateWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeOnInitialized() override;
	virtual void NativeDestruct() override;

	UFUNCTION(BlueprintCallable, Category = "Bind")
	void GameStateBinding();

protected:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Txt_Stage;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Txt_Timer;
	UPROPERTY(meta = (BindWidget))
	UProgressBar* Time_Bar;
	UPROPERTY(meta = (BindWidget))
	UImage* Img_Stage;

	UPROPERTY(EditAnywhere, Category = "Visual")
	TMap<EPCStageType, UTexture2D*> StageIcons;

private:
	TWeakObjectPtr<APCCombatGameState> PCGameState;
	FDelegateHandle RepHandle;
	FTimerHandle TickHandle;

	void ReFreshStatic();
	void TickUpdate();
};
