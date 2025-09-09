// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "DataAsset/FrameWork//PCStageData.h"
#include "PCCombatPlayerController.generated.h"


class UPCPlayerMainWidget;
class APCCombatBoard;
struct FGameplayTag;
class APCCarouselRing;
/**
 * 
 */
UCLASS()
class PROJECTPC_API APCCombatPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	APCCombatPlayerController();
	
protected:
	virtual void SetupInputComponent() override;
	virtual void BeginPlay() override;
	
private:
	// Player의 모든 Input에 대한 MappingContext, Action, Effect가 담긴 DataAsset
	UPROPERTY(EditDefaultsOnly, Category = "DataAsset", meta = (AllowPrivateAccess = "true"))
	class UPCDataAsset_PlayerInput* PlayerInputData;

#pragma region Move
	
private:
	FVector CachedDestination;
	float FollowTime;
	
	void OnInputStarted();
	void OnSetDestinationTriggered();
	void OnSetDestinationReleased();

#pragma endregion Move

#pragma region Shop
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ShopWidget")
	TSubclassOf<class UUserWidget> ShopWidgetClass;

	UPROPERTY()
	class UPCShopWidget* ShopWidget;

	UFUNCTION(BlueprintCallable)
	void LoadShopWidget();

	void ShopRequest_ShopRefresh();

	UFUNCTION(Server, Reliable)
	void Server_ShopRefresh();

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

	APCCombatBoard* FindBoardBySeatIndex(int32 BoardSeatIndex) const;
	
public:

	// 자기 보드 인덱스 저장
	UFUNCTION(Client, Reliable)
	void ClientSetHomeBoardIndex(int32 InHomeBoardIdx);

	// 보드 인덱스로 카메라 변경
	UFUNCTION(Client, Reliable)
	void ClientFocusBoardBySeatIndex(int32 BoardSeatIndex, bool bBattle, float Blend = 0.35f);
		
	// 서버->클라 : 임의 액터로 포커스 (회전초밥 등 사용)
	UFUNCTION(Client, Reliable)
	void ClientCameraSetCarousel(APCCarouselRing* CarouselRing, float BlendTime);


#pragma endregion Camera

#pragma region UI
public:

	UFUNCTION(BlueprintCallable)
	void EnsureMainHUDCreated();

	UFUNCTION(BlueprintCallable)
	void ShowWidget(bool bAnimate = true);

	UFUNCTION(BlueprintCallable)
	void HideWidget();

	UFUNCTION(Client, Reliable)
	void Client_ShowWidget();

	UFUNCTION(Client, Reliable)
	void Client_HideWidget();

private:
	
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UPCPlayerMainWidget> PlayerMainWidgetClass = nullptr;

	UPROPERTY()
	TObjectPtr<UPCPlayerMainWidget> PlayerMainWidget = nullptr;
#pragma endregion UI
};
