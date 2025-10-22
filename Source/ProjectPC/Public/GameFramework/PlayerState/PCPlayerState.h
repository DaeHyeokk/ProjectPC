// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "Shop/PCShopUnitData.h"
#include "GameplayTagContainer.h"
#include "PCPlayerState.generated.h"

class UPCSynergyComponent;
class UPCPlayerInventory;
DECLARE_MULTICAST_DELEGATE(FUnitDataInBoardUpdated);
DECLARE_MULTICAST_DELEGATE(FOnShopSlotsUpdated);
DECLARE_MULTICAST_DELEGATE(FOnWinningStreakUpdated);

class APCPlayerBoard;
class APCHeroUnitCharacter;
class UGameplayEffect;

/**
 * 
 */
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

	UFUNCTION(BlueprintCallable, Category = "UnitSpawn")
	void UnitSpawn(FGameplayTag UnitTag);

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

	void SetDisplayName_Server(const FString& InName);
	
	// OnRep 들은 위젯 갱신용(원하면 비워둬도 됨)
	UFUNCTION()
	void OnRep_LocalUserId() {}
	UFUNCTION()
	void OnRep_bIsLeader()   {}
	UFUNCTION()
	void OnRep_bIsReady()    {}
	UFUNCTION()
	void OnRep_SeatIndex();

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
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
	void ApplyRoundReward();

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
	
	FOnShopSlotsUpdated OnShopSlotsUpdated;
	
	void SetShopSlots(const TArray<FPCShopUnitData>& NewSlots);
	const TArray<FPCShopUnitData>& GetShopSlots();

	void ReturnAllUnitToShop();

#pragma endregion Shop

#pragma region Combat
	
private:
	UPROPERTY(ReplicatedUsing = OnRep_PlayerWinningStreak)
	int32 PlayerWinningStreak = 0;

	UFUNCTION()
	void OnRep_PlayerWinningStreak();

public:
	UFUNCTION()
	void PlayerWin();
	UFUNCTION()
	void PlayerLose();
	UFUNCTION()
	void PlayerResult();

	FOnWinningStreakUpdated OnWinningStreakUpdated;
	
	int32 GetPlayerWinningStreak() const;

#pragma endregion Combat

#pragma region Inventory

protected:
	UPROPERTY(VisibleDefaultsOnly, Category = "Inventory")
	UPCPlayerInventory* PlayerInventory;

public:
	FORCEINLINE UPCPlayerInventory* GetPlayerInventory() const { return PlayerInventory; }

	virtual bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags) override;
	
#pragma endregion Inventory

#pragma region Synergy

protected:
	UPROPERTY(VisibleAnywhere, Category="Synergy")
	TObjectPtr<UPCSynergyComponent> SynergyComponent;

public:
	UFUNCTION(BlueprintCallable, Category="Synergy")
	UPCSynergyComponent* GetSynergyComponent() const { return SynergyComponent; }
	
#pragma endregion Synergy
};
