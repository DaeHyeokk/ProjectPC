// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayEffectTypes.h"
#include "PCHeroStatusHoverPanel.generated.h"

class UPCUnitEquipmentComponent;
class APCCommonUnitCharacter;
class UHorizontalBox;
class UPCItemSlotWidget;
class UPCHeroUnitAbilitySystemComponent;
class UTextBlock;
class UProgressBar;
class UPCUnitAttributeSet;

UENUM()
enum class EHeroHoverStat : uint8
{
	MaxHP, CurHP, MaxMP, CurMP, ManaRegen,
	AD, Range, AS, ASInc, ASDec, PDef, MDef,
	PMul, MMul, CritChance, LifeSteal, DamageMul
};

/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCHeroStatusHoverPanel : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	void InitFromHero(APCCommonUnitCharacter* InHero);
	
	UFUNCTION(BlueprintCallable)
	void ShowPanelForHero(APCCommonUnitCharacter* InHero);

	UFUNCTION(BlueprintCallable)
	void HidePanel();
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
	UPROPERTY(Transient)
	TWeakObjectPtr<APCCommonUnitCharacter> CurHero;

	UPROPERTY(Transient)
	TWeakObjectPtr<UAbilitySystemComponent> ASC;

	TMap<FGameplayAttribute, EHeroHoverStat> AttrRoute;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UProgressBar> HealthBar = nullptr;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UProgressBar> ManaBar = nullptr;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> HealthText = nullptr;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> ManaText = nullptr;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> ManaRegenText = nullptr;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> AttackRangeText = nullptr;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> BaseDamageText = nullptr;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> AttackSpeedText = nullptr;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> PDefText = nullptr;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> MDefText = nullptr;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> PMultiplierText = nullptr;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> MMultiplierText = nullptr;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> CritChanceText = nullptr;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> LifeStealText = nullptr;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> DamageMultiplierText = nullptr;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UHorizontalBox> ItemSlotPanel = nullptr;

	UPROPERTY()
	TArray<TObjectPtr<UPCItemSlotWidget>> ItemSlotWidgets;

	TMap<FGameplayAttribute, FDelegateHandle> AttrChangedHandleMap;
	FDelegateHandle EquipItemChangedHandle;
	
	void BuildRoutes();

	void BindAll();
	void UnbindAll();
	
	// 최초 1회 전체 스탯 반영
	void ApplyAll() const;

	void OnAttrChanged(const FOnAttributeChangeData& Data);
	void OnEquipItemChanged() const;
	
	void UpdateHP() const;
	void UpdateMP() const;

	void UpdateEquipItemSlots() const;
	
	void UpdateText_Int(UTextBlock* TextBlock, const FGameplayAttribute& Attr) const;
	void UpdateText_F2(UTextBlock* TextBlock, const FGameplayAttribute& Attr) const;
	void UpdateText_Pct01(UTextBlock* TextBlock, const FGameplayAttribute& Attr) const; // 0~1 -> %
	void UpdateText_PctValue(UTextBlock* TextBlock, const FGameplayAttribute& Attr) const; // 0~100 -> 0~100%
	void UpdateText_RegenValue(UTextBlock* TextBlock, const FGameplayAttribute& Attr) const;
	
	static FText AsInt(const float Value)
	{
		return FText::AsNumber(FMath::RoundToInt(Value));
	}
	
	static FText AsFloat2(const float Value)
	{ 
	return FText::FromString(FString::Printf(TEXT("%.2f"), Value));
	}

	// 0~1 비율을 %로 (0.2 -> 20%)
	static FText AsPercent0_From01(float Ratio01)
	{
		const float Clamped = FMath::Clamp(Ratio01, 0.f, 1.f);
		return FText::FromString(FString::Printf(TEXT("%d%%"), FMath::RoundToInt(Clamped * 100.f)));
	}

	// % 값 표현 (20 -> 20%)
	static FText AsPercent0_FromPctValue(float PctValue)
	{
		const int32 Pct = FMath::RoundToInt(PctValue);
		return FText::FromString(FString::Printf(TEXT("%d%%"), Pct));
	}

	static FText AsRegenPerSec(const float Value)
	{
		return FText::FromString(FString::Printf(TEXT("%d/초"), FMath::RoundToInt(Value)));
	}
};
