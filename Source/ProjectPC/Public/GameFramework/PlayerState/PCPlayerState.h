// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "Shop/PCShopUnitData.h"
#include "GameplayTagContainer.h"
#include "PCPlayerState.generated.h"

/**
 * 
 */


DECLARE_MULTICAST_DELEGATE(FUnitDataInBoardUpdated);

UCLASS()
class PROJECTPC_API APCPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	APCPlayerState();
	
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

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// OnRep 들은 위젯 갱신용(원하면 비워둬도 됨)
	UFUNCTION() void OnRep_LocalUserId() {}
	UFUNCTION() void OnRep_bIsLeader()   {}
	UFUNCTION() void OnRep_bIsReady()    {}
	UFUNCTION() void OnRep_SeatIndex()   {}
#pragma endregion Login

#pragma region AbilitySystem
	
private:
	UPROPERTY(VisibleDefaultsOnly, Category = "AbilitySystem")
	class UPCPlayerAbilitySystemComponent* PlayerAbilitySystemComponent;

	UPROPERTY()
	class UPCPlayerAttributeSet* PlayerAttributeSet;
	
public:
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

#pragma endregion AbilitySystem

#pragma region Shop

public:
	UPROPERTY(ReplicatedUsing = OnRep_ShopSlots)
	TArray<FPCShopUnitData> ShopSlots;

	DECLARE_MULTICAST_DELEGATE(FOnShopSlotsUpdated);
	FOnShopSlotsUpdated OnShopSlotsUpdated;

	UFUNCTION()
	void OnRep_ShopSlots();

	void SetShopSlots(const TArray<FPCShopUnitData>& NewSlots);
	const TArray<FPCShopUnitData>& GetShopSlots();
	
#pragma endregion Shop
};
