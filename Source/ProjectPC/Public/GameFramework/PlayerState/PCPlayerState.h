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


// 보드 배치된 유닛의 데이터(임시)
USTRUCT(BlueprintType)
struct FUnitDataInBoard
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag UnitID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 UnitLevel = 1;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 TileIndex = -1;
	

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	uint8 TeamID = 0;

	FUnitDataInBoard(){}
	FUnitDataInBoard(FGameplayTag InID, int32 InLevel, int32 InIndex, uint8 InTeam) :
	UnitID(InID), UnitLevel(InLevel), TileIndex(InIndex), TeamID(InTeam){}
	
};

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

#pragma region MyFieldData
	// 필드/벤치 타일 데이터
public:

	UPROPERTY(Replicated, BlueprintReadOnly)
	uint8 TeamID = 0;
	
	UPROPERTY(ReplicatedUsing = OnRep_UnitData, BlueprintReadOnly, Category = "UnitData")
	TArray<FUnitDataInBoard> FieldUnit;

	UPROPERTY(ReplicatedUsing = OnRep_UnitData, BlueprintReadOnly, Category = "UnitData")
	TArray<FUnitDataInBoard> BenchUnit;

	// 배치 편의 API

	UFUNCTION(BlueprintCallable)
	void SetFieldUnit(FGameplayTag UnitID, int UnitLevel, int32 TileIndex);

	UFUNCTION(BlueprintCallable)
	void RemoveFieldAt(int32 TileIndex);

	UFUNCTION(BlueprintCallable)
	void SetBenchUnit(FGameplayTag UnitID, int UnitLevel, int32 BenchIndex);

	UFUNCTION(BlueprintCallable)
	void RemoveBench(int32 BenchIndex);

	// 브로드 캐스트
	FUnitDataInBoardUpdated OnBoardUpdated;
	

protected:
	UFUNCTION()
	void OnRep_UnitData();


#pragma endregion MyFieldData

#pragma region AbilitySystem
	
private:
	UPROPERTY(VisibleDefaultsOnly, Category = "AbilitySystem")
	class UPCPlayerAbilitySystemComponent* PlayerAbilitySystemComponent;

	UPROPERTY()
	const class UPCPlayerAttributeSet* PlayerAttributeSet;
	
public:
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	const UPCPlayerAttributeSet* GetAttributeSet() const;

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
