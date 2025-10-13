// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PCGameStateWidget.generated.h"

class UPCRoundCellWidget;
class UHorizontalBox;
class APCCombatGameState;
enum class EPCStageType : uint8;
class UImage;
class UProgressBar;
class UTextBlock;
/**
 * 
 */

USTRUCT()
struct FRoundChip
{
	GENERATED_BODY()
	UPROPERTY()
	UImage* Icon;

	UPROPERTY()
	UImage* Arrow;
};
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
	// UPROPERTY(meta = (BindWidget))
	// UHorizontalBox* HB_Rounds;
	//
	// // 아이콘 세트
	// UPROPERTY(EditAnywhere, Category = "Icons")
	// UPCStageIconData* IconData = nullptr;

	UPROPERTY(EditAnywhere, Category = "Visual")
	TMap<EPCStageType, UTexture2D*> StageIcons;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UPCRoundCellWidget> RoundCellClass;

	UPROPERTY(EditAnywhere)
	TArray<UPCRoundCellWidget*> Cells;
	
	int32 CachedRoundCount = 0;
	int32 CachedLitBoundary = -1;

private:
	TWeakObjectPtr<APCCombatGameState> PCGameState;
	FDelegateHandle RepHandle;
	FTimerHandle TickHandle;
	FDelegateHandle LayOutHandle;

	// 칩 캐시
	TArray<FRoundChip> Chips;

	void ReFreshStatic();
	void TickUpdate();

	// // 신규 : 라운드 트레커
	// void RebuildRoundChipsForStage(int32 StageIdx);
	// void UpdateRoundChipsState();
	//
	// // 아이콘 Resolve
	// UTexture2D* ResolveIconForRound(int32 StageIdx, int32 RoundIdx, bool bCurrent, bool bPastWin, bool bPastLose) const;
	//
	// // 헬퍼
	// void ClearHB();
	//
	// //void RebuildRoundCellsIfNeeded();
	// void UpdateArrow(bool bForce);
	
	
};
