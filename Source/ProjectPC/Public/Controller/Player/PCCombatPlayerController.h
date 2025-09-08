// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/PlayerController.h"
#include "DataAsset/FrameWork//PCStageData.h"
#include "PCCombatPlayerController.generated.h"

class APCCombatBoard;
class APCCarouselRing;
class UPCDataAsset_PlayerInput;
class UPCShopWidget;
class UUserWidget;

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
	
private:
	// Player의 모든 Input에 대한 MappingContext, Action, Effect가 담긴 DataAsset
	UPROPERTY(EditDefaultsOnly, Category = "DataAsset", meta = (AllowPrivateAccess = "true"))
	UPCDataAsset_PlayerInput* PlayerInputData;

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
	TSubclassOf<UUserWidget> ShopWidgetClass;

	UPROPERTY()
	UPCShopWidget* ShopWidget;

	UFUNCTION(BlueprintCallable)
	void LoadShopWidget();

	void ShopRequest_ShopRefresh();
	void ShopRequest_BuyXP();
	void ShopRequest_BuyUnit(FGameplayTag UnitTag, int32 SlotIndex);

	UFUNCTION(Server, Reliable)
	void Server_ShopRefresh();
	UFUNCTION(Server, Reliable)
	void Server_BuyXP();
	UFUNCTION(Server, Reliable)
	void Server_BuyUnit(FGameplayTag UnitTag, int32 SlotIndex);

#pragma endregion Shop


#pragma region Camera
	// 게임 카메라 세팅

private:
	UPROPERTY(Transient)
	int32 HomeBoardSeatIndex = INDEX_NONE;

	APCCombatBoard* FindBoardBySeatIndex(int32 BoardSeatIndex) const;
	
public:

	// 자기 보드 인덱스 저장
	UFUNCTION(Client, Reliable)
	void ClientSetHomeBoardIndex(int32 InHomeBoardIdx);

	UFUNCTION(Client, Reliable)
	void ClientFocusBoardBySeatIndex(int32 BoardSeatIndex, bool bRespectFlipPolicy, float Blend = 0.35f);

	UFUNCTION(BlueprintPure, Category = "Camera")
	bool ShouldFlipForBoardIndex(int32 BoardSeatIndex) const;
	
	// 서버->클라 : 내 자리 인덱스로 카메라 세팅
	UFUNCTION(Client, Reliable)
	void ClientCameraSet(int32 BoardIndex, float BlendTime);

	// 서버->클라 : 임의 액터로 포커스 (회전초밥 등 사용)
	UFUNCTION(Client, Reliable)
	void ClientCameraSetCarousel(APCCarouselRing* CarouselRing, float BlendTime);

	// 서버->클라 : 페이즈 변경 알림
	UFUNCTION(Client, Reliable)
	void ClientStageChanged(EPCStageType NewStage, const FString& StageRoundName, float Seconds);

#pragma endregion Camera
};
