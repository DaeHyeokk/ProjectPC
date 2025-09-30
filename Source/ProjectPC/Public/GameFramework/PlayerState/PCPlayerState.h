// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "BaseGameplayTags.h"
#include "Shop/PCShopUnitData.h"
#include "GameplayTagContainer.h"
#include "PCPlayerState.generated.h"

/**
 * 
 */


class APCPlayerBoard;
DECLARE_MULTICAST_DELEGATE(FUnitDataInBoardUpdated);

UCLASS()
class PROJECTPC_API APCPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	APCPlayerState();

	// 나의 PCPlayerBoard 캐시 (서버에서 세팅, 클라 복제 : 보드에 시각적 효과 적용 위함)
	UPROPERTY(BlueprintReadOnly, Replicated)
	APCPlayerBoard* PlayerBoard = nullptr;

	UFUNCTION(BlueprintCallable, Category = "PlayerBoard")
	FORCEINLINE APCPlayerBoard* GetPlayerBoard() { return PlayerBoard; }

	// 서버 전용 세터
	void SetPlayerBoard(APCPlayerBoard* InBoard);

	void ResolvePlayerBoardOnClient();
	
protected:
	virtual void BeginPlay() override;

#pragma region Login
	
public:
	// 로그인 ID (클라가 제출 → 서버가 확정/복제)
	UPROPERTY(ReplicatedUsing=OnRep_LocalUserId, BlueprintReadOnly)
	FString LocalUserId;

	UPROPERTY(ReplicatedUsing=OnRep_bIsLeader, BlueprintReadOnly)
	bool bIsLeader = false;

	UPROPERTY(ReplicatedUsing=OnRep_bIsReady, BlueprintReadOnly)
	bool bIsReady = false;

	UPROPERTY(ReplicatedUsing=OnRep_SeatIndex, BlueprintReadOnly)
	int32 SeatIndex = -1;

	UPROPERTY(Replicated)
	bool bIdentified = false;

	UPROPERTY(Replicated)
	int32 PlayerLevel = 30;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// OnRep 들은 위젯 갱신용(원하면 비워둬도 됨)
	UFUNCTION()
	void OnRep_LocalUserId() {}
	UFUNCTION()
	void OnRep_bIsLeader()   {}
	UFUNCTION()
	void OnRep_bIsReady()    {}
	UFUNCTION()
	void OnRep_SeatIndex();
#pragma endregion Login

#pragma region AbilitySystem
	
private:
	UPROPERTY(VisibleDefaultsOnly, Category = "AbilitySystem")
	class UPCPlayerAbilitySystemComponent* PlayerAbilitySystemComponent;

	UPROPERTY()
	const class UPCPlayerAttributeSet* PlayerAttributeSet;

	UPROPERTY()
	FGameplayTagContainer AllStateTags;
	
	UPROPERTY()
	FGameplayTag CurrentStateTag;
	
public:
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	const UPCPlayerAttributeSet* GetAttributeSet() const;
	FGameplayTag GetCurrentStateTag() const;

	UFUNCTION(BlueprintCallable)
	void ChangeState(FGameplayTag NewStateTag);

	void AddValueToPlayerStat(FGameplayTag PlayerStatTag, float Value) const;

#pragma endregion AbilitySystem

#pragma region Shop

private:
	UPROPERTY(ReplicatedUsing = OnRep_ShopSlots)
	TArray<FPCShopUnitData> ShopSlots;
	
	UFUNCTION()
	void OnRep_ShopSlots();
	
	UFUNCTION(Client, Reliable)
	void Client_ForceShopSlotsUpdate();

public:
	UPROPERTY()
	TSet<int32> PurchasedSlots;
	
	DECLARE_MULTICAST_DELEGATE(FOnShopSlotsUpdated);
	FOnShopSlotsUpdated OnShopSlotsUpdated;
	
	void SetShopSlots(const TArray<FPCShopUnitData>& NewSlots);
	const TArray<FPCShopUnitData>& GetShopSlots();

#pragma endregion Shop
};
