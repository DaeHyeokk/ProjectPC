// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Character.h"
#include "PCPlayerCharacter.generated.h"

class UPathFollowingComponent;

UCLASS()
class PROJECTPC_API APCPlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	APCPlayerCharacter();

protected:
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Tags")
	FGameplayTagContainer CharacterTags;
	
private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* TopDownCameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;
	
#pragma region PlayerDead
	
	UPROPERTY(Replicated)
	bool bIsDead;

public:
	UFUNCTION(BlueprintCallable)
	FORCEINLINE bool IsDead() const { return bIsDead; }

	void PlayerDie();

	UFUNCTION(BlueprintCallable)
	void OnPlayerDeathAnimFinished();

#pragma endregion PlayerDead

public:
	UFUNCTION(Client, Reliable)
	void Client_PlayMontage(UAnimMontage* Montage, float InPlayRate);
};
