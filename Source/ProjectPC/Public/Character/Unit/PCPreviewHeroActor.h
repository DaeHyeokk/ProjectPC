// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PCPreviewHeroActor.generated.h"

class UWidgetComponent;
class APCCombatBoard;
class APCHeroUnitCharacter;

UCLASS()
class PROJECTPC_API APCPreviewHeroActor : public AActor
{
	GENERATED_BODY()
	
public:	
	APCPreviewHeroActor();

	void InitializeFromSourceHero(
		APCHeroUnitCharacter* InSourceHero,
		const TSubclassOf<UAnimInstance>& SourcePreviewAnimBP,
		const TSubclassOf<UUserWidget>& SourceStatusBarClass);
	
	void UpdateLocation(const FVector& WorldLoc);
	UFUNCTION(BlueprintCallable)
	void TearDown();

	USkeletalMeshComponent* GetMesh() const { return MeshComponent; }
	UWidgetComponent* GetStatusBarComponent() const { return StatusBarComp; }
	
	UFUNCTION(BlueprintCallable)
	void SetupSourceSnapshot(struct FPoseSnapshot& OutSnapshot);
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USkeletalMeshComponent> MeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UWidgetComponent> StatusBarComp;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Source")
	TWeakObjectPtr<APCHeroUnitCharacter> SourceHero;
};
