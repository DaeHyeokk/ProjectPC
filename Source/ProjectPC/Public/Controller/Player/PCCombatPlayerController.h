// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseGameplayTags.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/HelpActor/PCPlayerBoard.h"
#include "PCCombatPlayerController.generated.h"


class APCBaseUnitCharacter;
class APCCombatBoard;
class APCCarouselRing;
class APCHeroUnitCharacter;
class UPCLoadingWidget;
class UPCPlayerInventoryWidget;
class UPCGameResultWidget;
class UPCTileManager;
class UPCDragComponent;
class UPCPlayerMainWidget;
class UPCDataAsset_PlayerInput;
class UPCShopWidget;
class UUserWidget;

/**
 * 
 */

// === 드래그 페이로드(서버 RPC에서만 사용) ===
USTRUCT()
struct FDragTile
{
	GENERATED_BODY()

	UPROPERTY() bool  bIsField = true;
	UPROPERTY() int32 Y = INDEX_NONE;
	UPROPERTY() int32 X = INDEX_NONE;
	UPROPERTY() int32 Bench = INDEX_NONE;
	UPROPERTY() int32 DragId = 0;
};

// ── 카메라 상태 enum 
UENUM(BlueprintType)
enum class ECameraFocusType : uint8
{
	None,
	Carousel,
	Board
};

UCLASS()
class PROJECTPC_API APCCombatPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	APCCombatPlayerController();

	UFUNCTION(Client, Reliable)
	void Client_RequestIdentity();

	UFUNCTION(Server, Reliable)
	void ServerSubmitIdentity(const FString& InDisplayName, const FGuid& InSessionID);
	
protected:
	virtual void SetupInputComponent() override;
	virtual void BeginPlay() override;
	virtual void BeginPlayingState() override;
	virtual void OnRep_PlayerState() override;
	virtual void AcknowledgePossession(APawn* P) override;

#pragma region Input
	
private:
	// Player의 모든 Input에 대한 MappingContext, Action이 담긴 DataAsset
	UPROPERTY(EditDefaultsOnly, Category = "DataAsset", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPCDataAsset_PlayerInput> PlayerInputData;
	
	FVector CachedDestination;
	float FollowTime;

	FTimerHandle MoveTimerHandle;
	
	// Move
	void OnInputStarted();
	void OnSetDestinationTriggered();
	void UpdateMovement();

	UFUNCTION(Server, Reliable)
	void Server_SetActorRotation(FRotator NewRotation);
	UFUNCTION(Server, Reliable)
	void Server_SetActorTransform(FTransform NewTransform);

public:
	UFUNCTION(Client, Reliable)
	void Client_StopMoving();

private:
	// Shop
	void OnBuyXPStarted();
	void OnShopRefreshStarted();
	void OnSellUnitStarted();

#pragma endregion Input

#pragma region Shop

private:
	bool bIsShopLocked = false;
	bool bIsShopRequestInProgress = false;
	
	UPROPERTY()
	TObjectPtr<UPCShopWidget> ShopWidget;

public:
	TArray<int32> GetSameShopSlotIndices(int32 SlotIndex);

	void ShopRequest_ShopRefresh(float GoldCost);
	void ShopRequest_BuyXP();
	void ShopRequest_SellUnit();
	void ShopRequest_BuyUnit(int32 SlotIndex);
	void ShopRequest_ShopLock(bool ShopLockState);

	UFUNCTION(Server, Reliable)
	void Server_ShopRefresh(float GoldCost);
	UFUNCTION(Server, Reliable)
	void Server_BuyXP();
	UFUNCTION(Server, Reliable)
	void Server_SellUnit(APCBaseUnitCharacter* Unit);
	UFUNCTION(Server, Reliable)
	void Server_BuyUnit(int32 SlotIndex);
	UFUNCTION(Server, Reliable)
	void Server_ShopLock(bool ShopLockState);
	
	UFUNCTION(Client, Reliable)
	void Client_SetSlotHidden(int32 SlotIndex);
	UFUNCTION(Client, Reliable)
	void Client_ShopRequestFinished();

#pragma endregion Shop

#pragma region Camera
	// 게임 카메라 세팅

protected:
	
	UPROPERTY(Transient)
	int32 HomeBoardSeatIndex = -1;

	// 현재 뷰 타깃 보드 SeatIndex ( 중복호출 방지용)
	UPROPERTY(Transient)
	int32 CurrentFocusedSeatIndex = -999;

	UPROPERTY()
	bool bBoardPresetInitialized = false;

	void SetBoardSpringArmPresets();
	
public:

	UPROPERTY()
	ECameraFocusType CurrentCameraType = ECameraFocusType::None;

	UPROPERTY()
	int32 CurrentCarouselSeatIndex = -1;

	UPROPERTY()
	int32 FocusedBoardSeatIndex = -1;
	
	// 자기 보드 인덱스 저장
	UFUNCTION(Client, Reliable)
	void ClientSetHomeBoardIndex(int32 InHomeBoardIdx);

	// 보드 인덱스로 카메라 변경
	UFUNCTION(Client, Reliable)
	void ClientFocusBoardBySeatIndex(int32 BoardSeatIndex, float Blend = 0.35f);
		
	// 서버->클라 캐러셀 카메라 세팅 (회전초밥 등 사용)
	UFUNCTION(Client, Reliable)
	void ClientCameraSetCarousel(APCCarouselRing* CarouselRing, int32 SeatIndex, float BlendTime);
	
	void SwitchCameraWhileBlack(AActor* NewTarget, float BlendTime, float FadeOutTime = 0.08f, float FadeInTime = 0.15f, float HoldBlack = 0.f);
	void ClearAllCameraTimers();

	FTimerHandle ThFadeSwitch;
	FTimerHandle ThFadeIn;
	bool bCameraFadeBusy = false;

	APCCombatBoard* FindBoardBySeatIndex(int32 BoardSeatIndex) const;

	// UI 가리기용 위젯
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> ScreenFadeClass;

	UPROPERTY()
	UUserWidget* ScreenFadeWidget = nullptr;

	void EnsureScreenFade();
	void SetScreenFadeVisible(bool bVisible, float Opacity = 1.f);

#pragma endregion Camera

#pragma region Loading
public:
	// 부트스트랩 플래그
	bool bPSReady = false;
	bool bPawnReady = false;
	bool bUIReady = false;
	bool bGSBound = false;

	// 로컬 풀링
	FTimerHandle ThBootstrapPing;
	void StartClientBootStrap();
	void TickClientBootStrap();
	
	// 게임스테이트 로딩 이벤트 구독
	UFUNCTION()
	void OnGameLoadingChanged();

	// 서버로 ACK ( 완료 시그널 전송 )
	UFUNCTION(Server,Reliable)
	void Server_ReportBootStrap(const FString& LocalUserId, uint8 Mask);

	// 동시시작 알림
	UFUNCTION()
	void HandlePreStartArmed();

private:

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UPCLoadingWidget> LoadingWidgetClass;

	UPROPERTY()
	UPCLoadingWidget* LoadingWidget = nullptr;
	
	// 헬퍼
	uint8 ComputeBootStrapMask() const;
	void ShowPlayerMainUI();
	void ShowLoadingUI();
	void UpdateLoadingUI(float Pct01, const FString& Line);
	void HideLoadingUI();

	
#pragma endregion

#pragma region UI
	
public:
	void LoadMainWidget();
	
	UFUNCTION(BlueprintCallable)
	void EnsureMainHUDCreated();

	void TryInitHUDWithPlayerState();

	UFUNCTION(Client, Reliable)
	void TryInitWidgetWithGameState();
	
	UFUNCTION(BlueprintCallable)
	void ShowShopWidget();
	UFUNCTION(BlueprintCallable)
	void HideShopWidget();

	UFUNCTION(Client, Reliable)
	void Client_ShowShopWidget();
	UFUNCTION(Client, Reliable)
	void Client_HideShopWidget();
	UFUNCTION(Client, Reliable)
	void Client_ShowPlayerMainWidget();

	UPCPlayerMainWidget* GetPlayerMainWidget() { return PlayerMainWidget; }

	// VFX
	UFUNCTION(Client, Reliable)
	void Client_PlaceFX(UNiagaraSystem* System, FVector Location, FRotator Rotation = FRotator::ZeroRotator);

private:
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UPCPlayerMainWidget> PlayerMainWidgetClass = nullptr;

	UPROPERTY()
	TObjectPtr<UPCPlayerMainWidget> PlayerMainWidget = nullptr;
	
#pragma endregion UI

#pragma region Drag&Drop

public:
	void ApplyGameInputMode();

	// === 드래그 RPC ===
public:
	// === 클라→서버: 월드 좌표 기반 드래그 RPC ===
	UFUNCTION(Server, Reliable)
	void Server_StartDragFromWorld(FVector World, int32 DragId);
	UFUNCTION(Server, Reliable)
	void Server_EndDrag(FVector World, int32 DragId);

	// Hover 이벤트 질의 / 응답
	FTimerHandle ThHoverPoll;

	UPROPERTY(EditAnywhere, Category = "Hover")
	float HoverPollHz = 15.f;

	UFUNCTION()
	void PollHover();

	UFUNCTION(Server, Unreliable)
	void Server_QueryHoverFromWorld(const FVector& World);
	
	UFUNCTION(Server, Unreliable)
	void Server_QueryTileUnit(bool bIsField, int32 Y, int32 X, int32 BenchIdx);

	UFUNCTION(Client, Reliable)
	void Client_CurrentDragUnit(APCBaseUnitCharacter* Unit);

	UFUNCTION(Client, Reliable)
	void Client_TileHoverUnit(APCBaseUnitCharacter* Unit);

	UFUNCTION(Client, Reliable)
	void Client_CheckHeroStatus(APCBaseUnitCharacter* Unit);

	UPROPERTY()
	TWeakObjectPtr<APCBaseUnitCharacter> CachedHoverUnit;
	TWeakObjectPtr<APCBaseUnitCharacter> GetCachedHoverUnit() const { return CachedHoverUnit; }

	UPROPERTY()
	TWeakObjectPtr<APCBaseUnitCharacter> CachedCheckStatusUnit;

	UPROPERTY()
	TWeakObjectPtr<APCHeroUnitCharacter> CachedPreviewUnit;

	// === 서버→클라(소유자): 피드백 ===
	UFUNCTION(Client, Unreliable)
	void Client_DragConfirm(bool bOk, int32 DragId, FVector StartSnap, APCHeroUnitCharacter* PreviewUnit = nullptr);
	UFUNCTION(Client, Unreliable)
	void Client_DragEndResult(bool bSuccess, FVector FinalSnap, int32 DragId, APCHeroUnitCharacter* PreviewUnit = nullptr);

	bool bIsCancel = false;
	void CancelDrag(const FGameplayTag& GameStateTag);

	UFUNCTION(Server,Reliable)
	void CancelDragServer();

	// 기존 바인딩 래퍼 (입력에서 호출)
	void OnMouse_Pressed();
	void OnMouse_Released();

	UPCTileManager* GetTileManager() const ;

	APCPlayerBoard* GetPlayerBoard() const;

	APCPlayerBoard* GetLocalPlayerBoard() const;
	
	
	// 외곽선 관련
	TWeakObjectPtr<APCBaseUnitCharacter> LastHoverUnit;

	// 드래그 중에는 하이라이트 고정
	bool bKeepDragHighlight = false;

	void SetHoverHighLight(APCBaseUnitCharacter* NewUnit);
	void ClearHoverHighLight();

	static bool IsBattleTag(const FGameplayTag& Tag)
	{
		return Tag.MatchesTagExact(GameStateTags::Game_State_Combat_Preparation) || Tag.MatchesTagExact(GameStateTags::Game_State_Combat_Active);
	}

	static bool IsBattleCreep(const FGameplayTag& Tag)
	{
		return Tag.MatchesTagExact(GameStateTags::Game_State_Combat_Preparation_Creep) || Tag.MatchesTagExact(GameStateTags::Game_State_Combat_Active_Creep);
	}


protected:
	// 서버 상태
	UPROPERTY(Transient) int32 CurrentDragId = 0;
	UPROPERTY(Transient) TWeakObjectPtr<APCBaseUnitCharacter> CurrentDragUnit;

	// === 서버 헬퍼 ===
	bool CanControlUnit(const APCBaseUnitCharacter* Unit) const;
	
	// 이동 연출(클라) - 서버에서 호출
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_LerpMove(APCBaseUnitCharacter* Unit, FVector Dest, float Duration);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UPCDragComponent* DragComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Drag")
	float LerpDuration = 0.15f;

#pragma endregion Drag&Drop

#pragma region GameResult

public:
	UPROPERTY(EditDefaultsOnly, Category = "GameResultWidget")
	TSubclassOf<UUserWidget> GameResultWidgetClass;

	UPROPERTY()
	TObjectPtr<UPCGameResultWidget> GameResultWidget;

	UFUNCTION(Client, Reliable)
	void Client_LoadGameResultWidget(int32 Ranking);

	void OnResultMenuToggled();

#pragma endregion GameResult

#pragma region Patrol

public:
	// 정찰용 시점 변환, 캐릭터 이동, 인벤토리 변경
	UFUNCTION(Client, Reliable)
	void Client_RequestPlayerReturn();
	
	void PlayerPatrol(APCPlayerState* OnPatrolPlayerState);
	void PlayerEndPatrol(bool IsPlayerTravel);

	void PatrolWidgetChange(APCPlayerState* OnPatrolPlayerState, bool IsOwner);
	void PatrolTransformChange(APCPlayerState* OnPatrolPlayerState, bool IsPlayerEndPatrol, bool IsPlayerTravel);

#pragma endregion Patrol
};
