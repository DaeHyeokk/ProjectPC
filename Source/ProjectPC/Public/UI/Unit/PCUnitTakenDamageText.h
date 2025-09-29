// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Blueprint/UserWidget.h"
#include "PCUnitTakenDamageText.generated.h"

/**
 * 
 */

class UImage;
class UTextBlock;
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDamageTextFinished);

UCLASS()
class PROJECTPC_API UPCUnitTakenDamageText : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category="DamageText")
	FOnDamageTextFinished OnFinished;

	UFUNCTION(Category="DamageText")
	void InitializeDamageText(const float DamageValue, const bool bIsCritical, const FGameplayTag& DamageTypeTag);
	
	UFUNCTION(BlueprintCallable, Category="DamageText")
	void NotifyFinished() { OnFinished.Broadcast(); }

protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> CriticalImage = nullptr;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> DamageText = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="TextStyle")
	FLinearColor PhysicalDamageColor = FLinearColor::Red;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="TextStyle")
	FLinearColor MagicDamageColor = FLinearColor::Blue;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="TextStyle")
	FLinearColor TrueDamageColor = FLinearColor::White;

	UPROPERTY(Transient, meta=(BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> DamageAnim;
	
};
