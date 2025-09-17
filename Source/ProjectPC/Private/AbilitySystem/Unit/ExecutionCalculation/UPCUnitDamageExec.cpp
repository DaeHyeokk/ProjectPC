// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Unit/ExecutionCalculation/UPCUnitDamageExec.h"
#include "BaseGameplayTags.h"
#include "AbilitySystem/Unit/AttributeSet/PCHeroUnitAttributeSet.h"
#include "AbilitySystem/Unit/AttributeSet/PCUnitAttributeSet.h"
#include "GameFramework/WorldSubsystem/PCUnitGERegistrySubsystem.h"

static FGameplayEffectAttributeCaptureDefinition MakeCapture(const FGameplayAttribute& Attr,
                                                             EGameplayEffectAttributeCaptureSource Source,
                                                             bool bSnapshot = false)
{
	FGameplayEffectAttributeCaptureDefinition Def;
	Def.AttributeToCapture = Attr;
	Def.AttributeSource = Source;
	Def.bSnapshot = bSnapshot;
	return Def;
}

UPCUnitDamageExec::UPCUnitDamageExec()
{
	// PhysicalDamageTypeTag = UnitGameplayTags::Unit_DamageType_Physical;
	// MagicDamageTypeTag = UnitGameplayTags::Unit_DamageType_Magic;
	//
	// DamageCallerTag = GameplayEffectTags::GE_Caller_Damage;
	// HealCallerTag = GameplayEffectTags::GE_Caller_Stat_CurrentHealth;
	// HealGEKeyTag = GameplayEffectTags::GE_Class_Health_Heal_Instant;

	Captures.PhysicalDefense = MakeCapture(UPCUnitAttributeSet::GetPhysicalDefenseAttribute(),
	EGameplayEffectAttributeCaptureSource::Target);
	Captures.MagicDefense = MakeCapture(UPCUnitAttributeSet::GetMagicDefenseAttribute(),
		EGameplayEffectAttributeCaptureSource::Target);

	// Hero 전용
	Captures.PhysDamageMultiplier = MakeCapture(UPCHeroUnitAttributeSet::GetPhysicalDamageMultiplierAttribute(),
		EGameplayEffectAttributeCaptureSource::Source);
	Captures.MagicDamageMultiplier = MakeCapture(UPCHeroUnitAttributeSet::GetMagicDamageMultiplierAttribute(),
		EGameplayEffectAttributeCaptureSource::Source);
	Captures.CritChance = MakeCapture(UPCHeroUnitAttributeSet::GetCritChanceAttribute(),
		EGameplayEffectAttributeCaptureSource::Source);
	Captures.CritMultiplier = MakeCapture(UPCHeroUnitAttributeSet::GetCritMultiplierAttribute(),
		EGameplayEffectAttributeCaptureSource::Source);
	Captures.LifeSteal = MakeCapture(UPCHeroUnitAttributeSet::GetLifeStealAttribute(),
		EGameplayEffectAttributeCaptureSource::Source);
	Captures.SpellVamp = MakeCapture(UPCHeroUnitAttributeSet::GetSpellVampAttribute(),
		EGameplayEffectAttributeCaptureSource::Source);

	// 이 Exec이 어떤 속성을 읽는지 시스템에 등록
	RelevantAttributesToCapture.Add(Captures.PhysicalDefense);
	RelevantAttributesToCapture.Add(Captures.MagicDefense);
	RelevantAttributesToCapture.Add(Captures.PhysDamageMultiplier);
	RelevantAttributesToCapture.Add(Captures.MagicDamageMultiplier);
	RelevantAttributesToCapture.Add(Captures.CritChance);
	RelevantAttributesToCapture.Add(Captures.CritMultiplier);
	RelevantAttributesToCapture.Add(Captures.LifeSteal);
	RelevantAttributesToCapture.Add(Captures.SpellVamp);
}

void UPCUnitDamageExec::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

	// BaseDamage: SetByCaller에서 읽음 (없으면 0)
	float BaseDamage = Spec.GetSetByCallerMagnitude(DamageCallerTag, 0.f);
	if (BaseDamage <= 0.f)
	{
		return; // 줄 데미지 없음
	}

	// DamageType 판정: DynamicAssetTags에서 확인
	const FGameplayTagContainer& DynTags = Spec.GetDynamicAssetTags();
	const bool bPhysical = DynTags.HasTag(PhysicalDamageTypeTag);
	const bool bMagic = DynTags.HasTag(MagicDamageTypeTag);
	const bool bUsePhysical = bPhysical || !bMagic;	// 기본은 물리로

	FAggregatorEvaluateParameters EvalParams;
	EvalParams.SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	EvalParams.TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	// 캡처 값 가져오기
	auto GetMagnitude = [&ExecutionParams, &EvalParams](const FGameplayEffectAttributeCaptureDefinition& Def, float& OutVal)
	{
		OutVal = 0.f;
		ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(Def, EvalParams, OutVal);
	};

	float Defense = 0.f;
	GetMagnitude(bUsePhysical ? Captures.PhysicalDefense : Captures.MagicDefense, Defense);

	// 타입 데미지 배율 (영웅 전용)
	float TypeMul = 0.f;
	GetMagnitude(bUsePhysical ? Captures.PhysDamageMultiplier : Captures.MagicDamageMultiplier, TypeMul);
	BaseDamage *= 1.f + TypeMul;

	// 치명타 (영웅 전용)
	float CritChance = 0.f, CritMul = 1.f;
	GetMagnitude(Captures.CritChance, CritChance);
	CritChance *= 0.01f;
	GetMagnitude(Captures.CritMultiplier, CritMul);
	CritMul += 1.f;
	
	if (CritChance > 0.f && CritMul > 1.f)
	{
		const UAbilitySystemComponent* TargetASC = ExecutionParams.GetTargetAbilitySystemComponent();
		
		// 서버에서만 판정
		if (TargetASC && TargetASC->GetOwner()->HasAuthority())
		{
			if (FMath::FRand() < FMath::Clamp(CritChance, 0.f, 1.f))
			{
				BaseDamage *= CritMul;
			}
		}
	}
	
	// 관통 적용 → 유효 저항 // 관통력 적용 임시 공식
	//const float EffectiveResist = FMath::Max(0.f, Resist - PenFlat) * (1.f - FMath::Clamp(PenPct, 0.f, 1.f));
	
	// 간단한 경감 공식
	const float Mitigation = 100.f / (100.f + Defense);
	const float FinalDamage = FMath::Max(0.f, BaseDamage * Mitigation);

	if (FinalDamage <= KINDA_SMALL_NUMBER)
	{
		return;
	}

	// Health에 음수로 적용
	OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(
		UPCUnitAttributeSet::GetCurrentHealthAttribute(),
		EGameplayModOp::Additive,
		-FinalDamage));

	// 피흡/주문흡혈 적용 (영웅 전용)
	float VampPct = 0.f;
	GetMagnitude(bUsePhysical ? Captures.LifeSteal : Captures.SpellVamp, VampPct);
	VampPct *= 0.01f;
	
	if (VampPct > 0.f)
	{
		const float HealAmount = FinalDamage * VampPct;
		if (HealAmount > KINDA_SMALL_NUMBER)
		{
			if (UAbilitySystemComponent* SourceASC = ExecutionParams.GetSourceAbilitySystemComponent())
			{
				if (const UGameplayEffect* HealGE = ResolveHealGE(SourceASC->GetWorld()))
				{
					const FGameplayEffectContextHandle Ctx = SourceASC->MakeEffectContext();
					const FGameplayEffectSpecHandle HealSpec = SourceASC->MakeOutgoingSpec(HealGE->GetClass(), 1, Ctx);
					if (HealSpec.IsValid())
					{
						HealSpec.Data->SetSetByCallerMagnitude(HealCallerTag, HealAmount);
						SourceASC->ApplyGameplayEffectSpecToSelf(*HealSpec.Data.Get());
					}
				}
			}
		}
	}
}

const UGameplayEffect* UPCUnitDamageExec::ResolveHealGE(const UWorld* World) const
{
	if (!World)
		return nullptr;

	if (UPCUnitGERegistrySubsystem* GERegistrySubsystem = World->GetSubsystem<UPCUnitGERegistrySubsystem>())
		return GERegistrySubsystem->GetGE_CDO(HealGEKeyTag);
	else
		return nullptr;
}
