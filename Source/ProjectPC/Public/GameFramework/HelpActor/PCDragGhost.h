// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PCDragGhost.generated.h"

UCLASS()
class PROJECTPC_API APCDragGhost : public AActor
{
	GENERATED_BODY()
	
public:	

	APCDragGhost();

	// 미리보기 업데이트
	UFUNCTION(BlueprintCallable)
	void UpdateState(bool bVisble, bool bValid, const FVector& Position);

	// 생성 헬퍼 (클라전용)
	static APCDragGhost* SpawnGhost(UWorld* World, const FVector& SpawnAt);

protected:

	virtual void BeginPlay() override;

private:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* StaticMesh;

	UPROPERTY()
	UMaterialInstanceDynamic* MID;

	FTimerHandle AutoDestoryTimer;
	void ArmAutoDestroy();
	void DestroySelf() { Destroy();}



	
	
};
