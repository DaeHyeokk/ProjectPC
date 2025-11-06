// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "PCGoldDisplayComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTPC_API UPCGoldDisplayComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	UPCGoldDisplayComponent();

	// 골드 표시할 매쉬
	UPROPERTY(EditAnywhere, Category = "GoldDisplay")
	TObjectPtr<UStaticMesh> MyGoldMesh;

	UPROPERTY(EditAnywhere, Category = "GoldDisplay")
	TObjectPtr<UStaticMesh> EnemyGoldMesh;

	UPROPERTY(EditAnywhere, Category = "GoldDisplay", meta=(ClampMin = 1))
	int32 GoldPerMesh = 10;

	// 첫번째 매쉬 기준 오프셋
	UPROPERTY(EditAnywhere, Category = "GoldDisplay")
	FVector StartOffset = FVector(-390,-890,-60);

	UPROPERTY(EditAnywhere, Category = "GoldDisplay")
	FVector EnemyStartOffset = FVector(410,890,-60);

	UPROPERTY(EditAnywhere, Category = "GoldDisplay")
	FRotator EnemyRotator = FRotator(0,-180,0);

	// 매쉬간 간격
	UPROPERTY(EditAnywhere, Category = "GoldDisplay")
	FVector Spacing = FVector(200,0,0);

	UPROPERTY(EditAnywhere, Category = "GoldDisplay")
	FVector EnemySpacing = FVector(-200,0,0);

	// 매쉬 최대 생성 개수
	UPROPERTY(EditAnywhere, Category = "GoldDisplay")
	int32 MaxPoolSize = 5;

	// 매쉬 스케일
	UPROPERTY(EditAnywhere, Category = "GoldDisplay")
	FVector MeshScale = FVector(0.4f);

	// 골드 업데이트
	UFUNCTION(BlueprintCallable, Category = "GoldDisplay")
	void UpdateFromMyGold(int32 PlayerGold);

	UFUNCTION(BlueprintCallable, Category = "GoldDisplay")
	void UpdateFromEnemyGold(int32 PlayerGold);

	UFUNCTION(BlueprintCallable, Category = "GoldDisplay")
	void ReSetMyDisplay();

	UFUNCTION(BlueprintCallable, Category = "GoldDisplay")
	void ReSetEnemyDisplay();

	

protected:
	virtual void OnRegister() override;

private:

	UPROPERTY(Transient)
	TArray<TObjectPtr<UStaticMeshComponent>> MyMeshPool;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UStaticMeshComponent>> EnemyMeshPool;

	void EnsureMyPool(int32 NeededCount);
	void EnsureEnemyPool(int32 NeededCount);
	void MyLayOutVisible(int32 VisibleCount);
	void EnemyLayOutVisible(int32 VisibleCount);

};
