// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "LobbyMenuWidget.generated.h"

class UCanvasPanel;
class UCanvasPanelSlot;
class UButton;
class UTextBlock;
class UBorder;
class UImage;
class UTexture2D;
class UPlayerDataWidget;
class APCPlayerState;

USTRUCT(BlueprintType)
struct FSeatLayout
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector2D Position = FVector2D::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector2D Size = FVector2D(450.f, 250.f);
};
/**
 * 
 */
UCLASS()
class PROJECTPC_API ULobbyMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

protected:

	UPROPERTY(meta = (BindWidget))
	UCanvasPanel* Canvas_Seats = nullptr;
	
	UPROPERTY(meta = (BindWidget))
	UButton* Btn_ToggleReady;

	UPROPERTY(meta = (BindWidget))
	UButton* Btn_Start;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Tb_ReadyCount;
	
protected:

	UFUNCTION()
	void OnClicked_ToggleReady();

	UFUNCTION()
	void OnClicked_Start();
	
	UFUNCTION(BLueprintCallable)
	void RefreshGrid();
	void EnsureSeatLayout(int32 NumSeats);

	void EnsureSeatFrames();

	int32 MapSeatToVisual(int32 ServerSeat, int32 LocalSeat, int32 NumSeats) const;

	UPlayerDataWidget* EnsureSeatWidget(int32 VisualIndex);

	void PlaceWidgetAt(int32 VisualIndex, UPlayerDataWidget* DataWidget);
	
	bool IsLeaderLocal() const;
	bool IsReadyLocal() const;

	void BuildOtherVisualOrder(TArray<int32>& OutOrder) const;

	

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Seat")
	TSubclassOf<UPlayerDataWidget> PlayerDataWidgetClass;

	UPROPERTY(EditAnywhere, Category = "Lobby|UI")
	TArray<FSeatLayout> SeatLayout;

	UPROPERTY(EditAnywhere, Category = "Lobby|UI")
	int32 CenterVisualIndex = 0;

	UPROPERTY(EditAnywhere, Category = "Lobby|UI")
	UTexture2D* SlotFrameTexture = nullptr;

	UPROPERTY(EditAnywhere, Category = "Lobby|UI")
	FMargin SlotFrameMargin = FMargin(0.15f);

	UPROPERTY(EditAnywhere, Category = "Lobby|UI")
	FLinearColor SlotFrameTint = FLinearColor::White;

	UPROPERTY(EditAnywhere, Category = "Lobby|UI")
	UTexture2D* EmptyPlusTexture = nullptr;

	UPROPERTY(EditAnywhere, Category = "Lobby|UI")
	TArray<int32> RelativeOrderToVisualIndex;

private:
	FTimerHandle RefreshTimer;

	FVector2D GetCanvasLayoutSize() const;

	UPROPERTY()
	TArray<TObjectPtr<UPlayerDataWidget>> SeatWidgets;

	UPROPERTY()
	TArray<TObjectPtr<UBorder>> SeatFrameBorders;

	UPROPERTY()
	TArray<TObjectPtr<UImage>> SeatEmptyIcons;
		
	
};
