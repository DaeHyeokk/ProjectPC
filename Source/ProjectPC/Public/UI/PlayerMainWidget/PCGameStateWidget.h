// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PCGameStateWidget.generated.h"

class UPCStepNoticeWidget;
class USizeBox;
class ISlateTextureAtlasInterface;
struct FStageIconVariant;
struct FGameplayTag;
class UPCWidgetIconData;
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

USTRUCT(BlueprintType)
struct FRoundChip
{
	GENERATED_BODY()
	
	UPROPERTY()
	TObjectPtr<UImage> Icon;

	UPROPERTY()
	TObjectPtr<UImage> Arrow;
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
	UPROPERTY(meta = (BindWidget))
	UHorizontalBox* HB_Rounds;

	UPROPERTY(meta = (BindWidget))
	USizeBox* SB_StateBar;

	UPROPERTY(meta = (BindWidget))
	USizeBox* SB_Timer;

	UPROPERTY(EditAnywhere, Category = "Layout")
	TScriptInterface<ISlateTextureAtlasInterface> Img_Arrow = nullptr;
	
	// 아이콘 세트
	UPROPERTY(EditAnywhere, Category = "Icons")
	UPCWidgetIconData* IconData = nullptr;

	UPROPERTY(EditAnywhere, Category = "Layout")
	FVector2D RoundIconSize = FVector2D(32.f, 32.f);

	UPROPERTY(EditAnywhere, Category = "Layout")
	FVector2D ArrowSize = FVector2D(40.f,40.f);
			
	int32 CachedStageIdx = -1;
	int32 CachedChipCount = 0;

private:

	
	TWeakObjectPtr<APCCombatGameState> PCGameState;
	FDelegateHandle RepHandle;
	FTimerHandle TickHandle;
	FDelegateHandle LayOutHandle;

	// 칩 캐시
	TArray<FRoundChip> Chips;

	void ReFreshStatic();
	void TickUpdate();
	

	// 레이아웃 체인지 핸들러
	void OnRoundsLayoutChanged_Handler();

	// 신규 : 라운드 트레커
	void RebuildRoundChipsForStage(int32 StageIdx);
	void UpdateRoundChipsState();
	
	// 아이콘 선택
	const FStageIconVariant* ChooseVariantFor(FGameplayTag Major, FGameplayTag PvESub) const;
	UTexture2D* PickIconFor(FGameplayTag Major,FGameplayTag PvESub, bool bCurrent, bool bPastWin, bool bPastLose) const;

	bool WasRoundVictory(int32 StageIdx, int32 RoundIdx) const {return false;}
	
	// 헬퍼
	void ClearHB();


protected:
	// 게임 스텝 노티스 위젯
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UPCStepNoticeWidget> W_StepNoticeWidget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Notice")
	FText SetUpText;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Notice")
	FText BattleText;

private:
	
	FDelegateHandle GameStateChangeHandle;
	void SetStepWidget(const FGameplayTag& GameStateTag);

	
	
	
};
