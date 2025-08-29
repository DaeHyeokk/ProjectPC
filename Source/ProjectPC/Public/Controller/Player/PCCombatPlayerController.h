// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/DataAsset/PCStageData.h"
#include "PCCombatPlayerController.generated.h"


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

	// 게임 카메라 세팅
public:
	// 서버->클라 : 내 자리 인덱스로 카메라 세팅
	UFUNCTION(Client, Reliable)
	void ClientCameraSet(int32 BoardIndex, float BlendTime);

	// 서버->클라 : 임의 액터로 포커스 (회전초밥 등 사용)
	UFUNCTION(Client, Reliable)
	void ClientCameraSetCarousel(APCCarouselRing* CarouselRing, float BlendTime);

	// 서버->클라 : 페이즈 변경 알림
	UFUNCTION(Client, Reliable)
	void ClientStageChanged(EPCStageType NewStage, const FString& StageRoundName, float Seconds);
};
