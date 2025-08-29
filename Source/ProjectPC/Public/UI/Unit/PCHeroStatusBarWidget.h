// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "Blueprint/UserWidget.h"
#include "PCHeroStatusBarWidget.generated.h"

class UPCUnitStatusBarWidget;
class UWidgetSwitcher;
/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCHeroStatusBarWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	// ASC와 4개 속성 초기화, Mana 속성은 Hero만 있고 Creep은 없으므로 매개변수 기본값 적용
	UFUNCTION()
	void InitWithASC(UAbilitySystemComponent* InASC,
		FGameplayAttribute InHealthAttr,
		FGameplayAttribute InMaxHealthAttr,
		FGameplayAttribute InManaAttr = FGameplayAttribute(),
		FGameplayAttribute InMaxManaAttr = FGameplayAttribute(),
		int32 InLevel = 1);

	UFUNCTION()
	void SetVariantByLevel(int32 Level);

protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UWidgetSwitcher> StatusSwitcher;
	
private:
	void EnsureActiveVariantInitialized();

	TWeakObjectPtr<UAbilitySystemComponent> ASC;
	FGameplayAttribute HealthAttr;
	FGameplayAttribute MaxHealthAttr;
	FGameplayAttribute ManaAttr;
	FGameplayAttribute MaxManaAttr;
	
	TSet<TWeakObjectPtr<UPCUnitStatusBarWidget>> InitializedStatusBarSet;
};
