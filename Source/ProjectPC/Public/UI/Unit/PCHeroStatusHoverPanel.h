// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayEffectTypes.h"
#include "Serialization/ArchiveReplaceObjectRef.h"
#include "PCHeroStatusHoverPanel.generated.h"

UENUM()
enum class EHeroHoverStat : uint8
{
	MaxHP, CurHP, MaxMP, CurMP,
	AD, Range, AS, PDef, MDef,
	PMul, MMul, CritChance, LifeSteal, SpellVamp
};

class FProperty;
class UPCHeroUnitAbilitySystemComponent;
class APCHeroUnitCharacter;
class UTextBlock;
class UProgressBar;
class UPCUnitAttributeSet;
/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCHeroStatusHoverPanel : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	void InitFromHero(APCHeroUnitCharacter* InHero);

	// 유닛이 호버 될 때 UI를 띄우면서 UI 갱신
	UFUNCTION(BlueprintCallable)
	void ShowPanelForHero(APCHeroUnitCharacter* InHero);

	UFUNCTION(BlueprintCallable)
	void HidePanel();
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
	UPROPERTY(Transient)
	TWeakObjectPtr<APCHeroUnitCharacter> Hero;
	UPROPERTY(Transient)
	TWeakObjectPtr<UPCHeroUnitAbilitySystemComponent> HeroASC;

	TMap<FGameplayAttribute, EHeroHoverStat> AttrRoute;
	TMap<FGameplayAttribute, FDelegateHandle> HandleMap;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UProgressBar> HealthBar = nullptr;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UProgressBar> ManaBar = nullptr;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> HealthText = nullptr;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> ManaText = nullptr;

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
	TObjectPtr<UTextBlock> SpellVampText = nullptr;

	void BuildRoutes();

	void BindAll();
	void UnbindAll();
	
	// 최초 1회 전체 스탯 반영
	void ApplyAll() const;

	void OnAttrChanged(const FOnAttributeChangeData& Data);

	void UpdateHP() const;
	void UpdateMP() const;

	void UpdateText_Int(UTextBlock* TextBlock, const FGameplayAttribute& Attr) const;
	void UpdateText_F2(UTextBlock* TextBlock, const FGameplayAttribute& Attr) const;
	void UpdateText_Pct01(UTextBlock* TextBlock, const FGameplayAttribute& Attr) const; // 0~1 -> %
	void UpdateText_PctMul(UTextBlock* TextBlock, const FGameplayAttribute& Attr) const; // 1.x -> %

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
		const float Clamped = FMath::Clamp(Ratio01, 0.1, 1.f);
		return FText::FromString(FString::Printf(TEXT("%d%%"), FMath::RoundToInt(Clamped * 100.f)));
	}

	// 배수의 증감만 표현 (1.2 -> 20%)
	static FText AsPercent0_FromMul(float Multiplier)
	{
		const float Delta01 = Multiplier - 1.f;
		const int32 Pct = FMath::RoundToInt(Delta01 * 100.f);
		return FText::FromString(FString::Printf(TEXT("%d%%"), Pct));
	}
};
