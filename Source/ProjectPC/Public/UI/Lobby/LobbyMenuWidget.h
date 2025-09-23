// LobbyMenuWidget.h
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LobbyMenuWidget.generated.h"

class UCenterPlayerWidget;
class UButton;
class UTextBlock;
class UPanelWidget;     // Border/Overlay/Canvas 모두 커버
class UOverlay;
class UWidget;
class UPlayerDataWidget;
class APCPlayerState;

UCLASS()
class PROJECTPC_API ULobbyMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// -------- UUserWidget ----------
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/** 즉시 새로고침하고 싶을 때 호출 */
	UFUNCTION(BlueprintCallable, Category="Lobby")
	void RefreshGrid();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lobby")
	TSubclassOf<UCenterPlayerWidget> CenterPlayerWidgetClass;

protected:
	// ============== 위젯 참조(디자이너 바인딩) ==============
	// 버튼/텍스트
	UPROPERTY(meta=(BindWidgetOptional))
	UButton* Btn_ToggleReady = nullptr;

	UPROPERTY(meta=(BindWidgetOptional))
	UButton* Btn_Start = nullptr;

	UPROPERTY(meta=(BindWidgetOptional))
	UTextBlock* Tb_ReadyCount = nullptr;

	// 슬롯 패널들 — 디자이너에서 Border/Overlay/Canvas 무엇이든 가능
	// 이름은 그대로 맞춰두면 자동 바인딩됨(없으면 nullptr)
	UPROPERTY(meta=(BindWidgetOptional)) UPanelWidget* Slot_Center = nullptr; // 0번(항상 로컬)
	UPROPERTY(meta=(BindWidgetOptional)) UPanelWidget* Slot_1     = nullptr;
	UPROPERTY(meta=(BindWidgetOptional)) UPanelWidget* Slot_2     = nullptr;
	UPROPERTY(meta=(BindWidgetOptional)) UPanelWidget* Slot_3     = nullptr;
	UPROPERTY(meta=(BindWidgetOptional)) UPanelWidget* Slot_4     = nullptr;
	UPROPERTY(meta=(BindWidgetOptional)) UPanelWidget* Slot_5     = nullptr;
	UPROPERTY(meta=(BindWidgetOptional)) UPanelWidget* Slot_6     = nullptr;
	UPROPERTY(meta=(BindWidgetOptional)) UPanelWidget* Slot_7     = nullptr;

	// 생성할 플레이어 타일 위젯 클래스
	UPROPERTY(EditDefaultsOnly, Category="Lobby|Classes")
	TSubclassOf<UPlayerDataWidget> PlayerDataWidgetClass;

	// 자동 새로고침
	UPROPERTY(EditAnywhere, Category="Lobby|Refresh")
	bool bAutoRefresh = true;

	UPROPERTY(EditAnywhere, Category="Lobby|Refresh", meta=(ClampMin="0.05", UIMin="0.05"))
	float RefreshIntervalSec = 0.30f;

private:
	// ============== 내부 상태 ==============
	// 디자이너에서 실제로 존재하는 슬롯만 모음(중앙+1..7)
	UPROPERTY(Transient)
	TArray<UPanelWidget*> SeatSlots;
	
	// 슬롯별로 생성된 플레이어 타일
	UPROPERTY(Transient)
	TArray<TWeakObjectPtr<UUserWidget>> SeatWidgets;

	FTimerHandle RefreshTimer;

	// ============== 내부 헬퍼 ==============
	void CollectSeatSlots();               // 디자이너 슬롯 수집
	void BuildOtherVisualOrder(TArray<int32>& OutOrder) const; // 1..N 순서
	void ClearAllSeats();
	void ClearSeat(int32 Index);

	/** 슬롯에 기존 컨텐츠가 있어도 위에 겹쳐서 넣어주는 안전한 Add (Border 등 포함) */
	void AddChildStacked(UPanelWidget* Panel, UWidget* Child);

	/** 필요 시 새로 만들어 반환 */
	UWidget* CreateOrGetSeatWidget(int32 Index);

	/** 인덱스 슬롯을 해당 PlayerState의 타일로 채움 */
	void SetSeatOccupied(int32 Index, APCPlayerState* PS);

	// 버튼 콜백
	UFUNCTION() void OnClicked_ToggleReady();
	UFUNCTION() void OnClicked_Start();

	// 로컬 상태
	bool IsLeaderLocal() const;
	bool IsReadyLocal() const;
};
