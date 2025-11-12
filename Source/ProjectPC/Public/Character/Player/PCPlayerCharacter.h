// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Character.h"
#include "PCPlayerCharacter.generated.h"

class APCCarouselRing;
class APCCarouselHeroCharacter;
class UWidgetComponent;

USTRUCT(BlueprintType)
struct FCarouselUnitData
{
	GENERATED_BODY()

	UPROPERTY()
	FGameplayTag UnitTag;

	UPROPERTY()
	FGameplayTag ItemTag;
};

UCLASS()
class PROJECTPC_API APCPlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	APCPlayerCharacter();

protected:
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	UPROPERTY(EditDefaultsOnly, Category = "Tags")
	FGameplayTagContainer CharacterTags;

#pragma region OverHeadWidget
	
protected:
	UPROPERTY(EditDefaultsOnly, Category = "OverHeadUI")
	TObjectPtr<UWidgetComponent> OverHeadWidgetComp;
	
public:
	void SetOverHeadWidgetPosition(FGameplayTag PlayerStateTag);

	// UFUNCTION(Client, Reliable)
	// void Multicast_SetOverHeadWidget();

#pragma endregion OverHeadWidget

#pragma region PlayerDead

private:
	UPROPERTY(Replicated)
	bool bIsDead;

public:
	UFUNCTION(BlueprintCallable)
	FORCEINLINE bool IsDead() const { return bIsDead; }

	void PlayerDie();

	UFUNCTION(Client, Reliable)
	void Client_PlayMontage(UAnimMontage* Montage, float InPlayRate);
	
	UFUNCTION(BlueprintCallable)
	void OnPlayerDeathAnimFinished();

#pragma endregion PlayerDead

#pragma region CarouselSlot

public:
	UFUNCTION(Server, Reliable)
	void Server_RequestCarouselPick();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Carousel")
	TWeakObjectPtr<APCCarouselRing> CarouselRing;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Carousel")
	USceneComponent* CarrySlot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Carousel")
	TWeakObjectPtr<APCCarouselHeroCharacter> CachedCarouselUnit;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Carousel")
	FCarouselUnitData CarouselUnitData;

	UFUNCTION(BlueprintCallable, Category = "Carousel")
	void CarouselUnitToSpawn();

#pragma endregion  CrouselSlot
};
