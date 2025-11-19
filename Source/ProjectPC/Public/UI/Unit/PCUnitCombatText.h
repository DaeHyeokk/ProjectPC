// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Blueprint/UserWidget.h"
#include "PCUnitCombatText.generated.h"

/**
 * 
 */

class UImage;
class UTextBlock;
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCombatTextFinished);

UCLASS()
class PROJECTPC_API UPCUnitCombatText : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category="DamageText")
	FOnCombatTextFinished OnFinished;
	
	UFUNCTION(Category="Combat|Damage")
	void InitializeDamageText(const float DamageValue, const bool bIsCritical, const FGameplayTag& DamageTypeTag);

	UFUNCTION(Category="Combat|Heal")
	void InitializeHealText(const float HealValue);

	UFUNCTION(Category="Combat|Miss")
	void InitializeMissText();
	
	UFUNCTION(BlueprintCallable, Category="DamageText")
	void NotifyFinished() const { OnFinished.Broadcast(); }

protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> CriticalImage = nullptr;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> ValueText = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="TextStyle")
	FLinearColor PhysicalDamageColor = FLinearColor::Red;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="TextStyle")
	FLinearColor MagicDamageColor = FLinearColor::Blue;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="TextStyle")
	FLinearColor TrueDamageColor = FLinearColor::White;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="TextStyle")
	FLinearColor HealColor = FLinearColor::Green;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="TextStyle")
	FLinearColor MissColor = FLinearColor::Gray;
	
	UPROPERTY(Transient, meta=(BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> DamageAnim;

	UPROPERTY(Transient, meta=(BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> HealAnim;
};
