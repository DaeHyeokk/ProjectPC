// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "Blueprint/UserWidget.h"
#include "PCUnitStatusBarWidget.generated.h"


class UTextBlock;
class UProgressBar;
class UPCUnitHealthProgressBar;
struct FOnAttributeChangeData;
class UAbilitySystemComponent;
/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCUnitStatusBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// ASC와 4개 속성 초기화, Mana 속성은 Hero만 있고 Creep은 없으므로 매개변수 기본값 적용
	UFUNCTION()
	void InitWithASC(UAbilitySystemComponent* InASC,
		FGameplayAttribute InHealthAttr,
		FGameplayAttribute InMaxHealthAttr,
		FGameplayAttribute InManaAttr = FGameplayAttribute(),
		FGameplayAttribute InMaxManaAttr = FGameplayAttribute());

	// ASC 없이 수동 갱신하고 싶을 때
	UFUNCTION()
	void SetInstant(float CurrentHP, float MaxHP, float CurrentMP = 0.f, float MaxMP = 0.f);

protected:
	virtual void NativeDestruct() override;

private:
	UPROPERTY(meta=(BindWidget))
	UPCUnitHealthProgressBar* HealthBar = nullptr;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* HealthText = nullptr;
	
	UPROPERTY(meta=(BindWidget))
	UProgressBar* ManaBar = nullptr;

	TWeakObjectPtr<UAbilitySystemComponent> ASC;
	FGameplayAttribute HealthAttr;
	FGameplayAttribute MaxHealthAttr;
	FGameplayAttribute ManaAttr;
	FGameplayAttribute MaxManaAttr;

	float CachedHP = 0.f;
	float CachedMaxHP = 1.f;
	float CachedMP = 0.f;
	float CachedMaxMP = 1.f;
	bool bHasMana = false;

	FDelegateHandle HealthHandle;
	FDelegateHandle MaxHealthHandle;
	FDelegateHandle ManaHandle;
	FDelegateHandle MaxManaHandle;
	
	void OnHealthChanged(const FOnAttributeChangeData& Data);
	void OnMaxHealthChanged(const FOnAttributeChangeData& Data);
	void OnManaChanged(const FOnAttributeChangeData& Data);
	void OnMaxManaChanged(const FOnAttributeChangeData& Data);
	
	void ApplyToUI() const;
};
