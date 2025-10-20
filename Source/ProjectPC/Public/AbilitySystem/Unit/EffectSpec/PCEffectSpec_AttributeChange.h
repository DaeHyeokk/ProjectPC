// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "PCEffectSpec_SetByCaller.h"
#include "ScalableFloat.h"
#include "PCEffectSpec_AttributeChange.generated.h"

/**
 * 
 */
UENUM(BlueprintType)
enum class EMagnitudeMode : uint8
{
	Constant,       // 상수 (고정 효과)
	Scalable,       // FScalableFloat (레벨 의존)
};

USTRUCT(BlueprintType)
struct FMagnitudeParam
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EMagnitudeMode Mode = EMagnitudeMode::Constant;

	// Constant
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(EditCondition="Mode==EMagnitudeMode::Constant", EditConditionHides))
	float ConstantMagnitude = 0.f;

	// Scalable
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(EditCondition="Mode==EMagnitudeMode::Scalable", EditConditionHides))
	FScalableFloat ScalableMagnitude;

	// 평가 함수: 필요하면 레벨
	float Evaluate(const int32 Level = 1) const
	{
		switch (Mode)
		{
		case EMagnitudeMode::Constant:
			return ConstantMagnitude;

		case EMagnitudeMode::Scalable:
			return ScalableMagnitude.GetValueAtLevel(FMath::Max(Level, 1));

		default:
			return 0.f;
		}
	}

	bool IsValid() const
	{
		return Evaluate() > 0.f;
	}
};

UCLASS()
class PROJECTPC_API UPCEffectSpec_AttributeChange : public UPCEffectSpec_SetByCaller
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, Category="Effect")
	FMagnitudeParam EffectMagnitude;
	
	virtual FActiveGameplayEffectHandle ApplyEffectImpl(UAbilitySystemComponent* SourceASC, const AActor* Target, int32 EffectLevel) override;
	
};
