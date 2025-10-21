// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "Blueprint/UserWidget.h"
#include "Character/Unit/PCBaseUnitCharacter.h"
#include "PCUnitStatusBarWidget.generated.h"


class UImage;
class UPCItemSlotWidget;
class UHorizontalBox;
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
	void InitWithASC(APCBaseUnitCharacter* InUnit, UAbilitySystemComponent* InASC,
		FGameplayAttribute InHealthAttr,
		FGameplayAttribute InMaxHealthAttr,
		FGameplayAttribute InManaAttr = FGameplayAttribute(),
		FGameplayAttribute InMaxManaAttr = FGameplayAttribute());

	// ASC 없이 수동 갱신하고 싶을 때
	void SetInstant(APCBaseUnitCharacter* InUnit,
		float CurrentHP, float MaxHP, float CurrentMP = 0.f, float MaxMP = 0.f);
	void UpdateUI() const;
	void ClearDelegate();

	FORCEINLINE void SetOwnerUnit(APCBaseUnitCharacter* InUnit) { Unit = InUnit; }

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UPCUnitHealthProgressBar> HealthBar;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UProgressBar> ManaBar;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> ItemImage_0;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> ItemImage_1;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> ItemImage_2;
	
	TWeakObjectPtr<APCBaseUnitCharacter> Unit;
	TWeakObjectPtr<UAbilitySystemComponent> ASC;
	FGameplayAttribute HealthAttr;
	FGameplayAttribute MaxHealthAttr;
	FGameplayAttribute ManaAttr;
	FGameplayAttribute MaxManaAttr;

	float CachedHP = 0.f;
	float CachedMaxHP = 1.f;
	float CachedMP = 0.f;
	float CachedMaxMP = 1.f;
	bool bHasMana = true;

	FDelegateHandle HealthHandle;
	FDelegateHandle MaxHealthHandle;
	FDelegateHandle ManaHandle;
	FDelegateHandle MaxManaHandle;

	UPROPERTY()
	TArray<TObjectPtr<UImage>> EquipItemImages;

	FDelegateHandle EquipItemChangedHandle;
	
	void OnHealthChanged(const FOnAttributeChangeData& Data);
	void OnMaxHealthChanged(const FOnAttributeChangeData& Data);
	void OnManaChanged(const FOnAttributeChangeData& Data);
	void OnMaxManaChanged(const FOnAttributeChangeData& Data);

	void OnEquipItemChanged() const;
	
	void ApplyToUI() const;
	void UpdateHealthBar() const;
	void UpdateManaBar() const;
	void UpdateEquipItemImages() const;
};
