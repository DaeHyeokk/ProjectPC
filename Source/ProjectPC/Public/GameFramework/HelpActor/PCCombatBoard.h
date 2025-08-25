// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PCCombatBoard.generated.h"

class UCameraComponent;
class USpringArmComponent;

UCLASS()
class PROJECTPC_API APCCombatBoard : public AActor
{
	GENERATED_BODY()
	
public:	
	APCCombatBoard();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	USceneComponent* SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UCameraComponent* BoardCamera;

	// 해당 보드 번호 (SeatIndex와 1:1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Board")
	int32 BoardSeatIndex = 0;

	// SeatAnchor 소켓 부모를 에디터에서 지정
	UPROPERTY(EditAnywhere, Category = "Seat")
	FComponentReference PlayerSeatParent;

	UPROPERTY(EditAnywhere, Category = "Seat")
	FComponentReference EnemySeatParent;

	UPROPERTY(EditAnywhere, Category = "Seat")
	FName PlayerSeatSocket = TEXT("SeatPlayer_Socket");

	UPROPERTY(EditAnywhere, Category = "Seat")
	FName EnemySeatSocket = TEXT("SeatEnemy_Socket");

	UPROPERTY(EditAnywhere, Category = "Seat")
	FVector SeatExtraOffset = FVector(0,0,0);

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Seat")
	USceneComponent* PlayerSeatAnchor = nullptr;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Seat")
	USceneComponent* EnemySeatAnchor = nullptr;

	UFUNCTION(BlueprintCallable)
	FTransform GetPlayerSeatTransform() const;

	UFUNCTION(BlueprintCallable)
	FTransform GetEnemySeatTransform() const;

	// 로컬 플레이어 카메라
	UFUNCTION(BlueprintCallable)
	void ApplyLocalBottomView(class APlayerController* PlayerController, int32 MySeatIndex, float Blend = 0.35f);

protected:
	
	virtual void BeginPlay() override;

#if WITH_EDITOR
	virtual void OnConstruction(const FTransform& Transform) override;
#endif

	void RebuildAnchors();
	USceneComponent* Resolve(const FComponentReference& Ref) const;

	
	
};
