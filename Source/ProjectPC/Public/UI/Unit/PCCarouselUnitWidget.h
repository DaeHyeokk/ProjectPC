// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PCCarouselUnitWidget.generated.h"

class APCCommonUnitCharacter;
class APCCarouselHeroCharacter;
struct FGameplayTag;
class UButton;
class UImage;
/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCCarouselUnitWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:

	UPROPERTY()
	TWeakObjectPtr<APCCommonUnitCharacter> CachedHero;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UImage* ItemImg;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* UnitInfoButton;

	UFUNCTION()
	void SetItemImg(FGameplayTag ItemTag);

	UFUNCTION()
	void SetUnit(APCCommonUnitCharacter* Unit);
	
	UFUNCTION()
	void CreateUnitInfoWidget();

private:
	virtual void NativeConstruct() override;
	
};
