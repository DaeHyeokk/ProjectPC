// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Unit/ExecutionCalculation/PCUnitDamageExec.h"
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
	Captures.PhysicalDefense = MakeCapture(UPCUnitAttributeSet::GetPhysicalDefenseAttribute(),
		EGameplayEffectAttributeCaptureSource::Target);
	Captures.MagicDefense = MakeCapture(UPCUnitAttributeSet::GetMagicDefenseAttribute(),
		EGameplayEffectAttributeCaptureSource::Target);
	Captures.Evasion = MakeCapture(UPCUnitAttributeSet::GetEvasionChanceAttribute(),
		EGameplayEffectAttributeCaptureSource::Target);
	
	Captures.FlatDamageBlock = MakeCapture(UPCUnitAttributeSet::GetFlatDamageBlockAttribute(),
		EGameplayEffectAttributeCaptureSource::Target);
	
	// Hero 전용
	Captures.PhysDamageMultiplier = MakeCapture(UPCHeroUnitAttributeSet::GetPhysicalDamageMultiplierAttribute(),
		EGameplayEffectAttributeCaptureSource::Source);
	Captures.MagicDamageMultiplier = MakeCapture(UPCHeroUnitAttributeSet::GetMagicDamageMultiplierAttribute(),
		EGameplayEffectAttributeCaptureSource::Source);
	Captures.DamageMultiplier = MakeCapture(UPCHeroUnitAttributeSet::GetDamageMultiplierAttribute(),
		EGameplayEffectAttributeCaptureSource::Source);
	Captures.CritChance = MakeCapture(UPCHeroUnitAttributeSet::GetCritChanceAttribute(),
		EGameplayEffectAttributeCaptureSource::Source);
	Captures.CritMultiplier = MakeCapture(UPCHeroUnitAttributeSet::GetCritMultiplierAttribute(),
		EGameplayEffectAttributeCaptureSource::Source);
	Captures.LifeSteal = MakeCapture(UPCHeroUnitAttributeSet::GetLifeStealAttribute(),
		EGameplayEffectAttributeCaptureSource::Source);

	// 이 Exec이 어떤 속성을 읽는지 시스템에 등록
	RelevantAttributesToCapture.Add(Captures.PhysicalDefense);
	RelevantAttributesToCapture.Add(Captures.MagicDefense);
	RelevantAttributesToCapture.Add(Captures.Evasion);

	RelevantAttributesToCapture.Add(Captures.FlatDamageBlock);
	RelevantAttributesToCapture.Add(Captures.IncomingPhysicalDamageMultiplier);
	RelevantAttributesToCapture.Add(Captures.IncomingMagicDamageMultiplier);
	
	RelevantAttributesToCapture.Add(Captures.PhysDamageMultiplier);
	RelevantAttributesToCapture.Add(Captures.MagicDamageMultiplier);
	RelevantAttributesToCapture.Add(Captures.DamageMultiplier);
	RelevantAttributesToCapture.Add(Captures.CritChance);
	RelevantAttributesToCapture.Add(Captures.CritMultiplier);
	RelevantAttributesToCapture.Add(Captures.LifeSteal);
}

void UPCUnitDamageExec::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	UAbilitySystemComponent* TargetASC = ExecutionParams.GetTargetAbilitySystemComponent();
	UAbilitySystemComponent* SourceASC = ExecutionParams.GetSourceAbilitySystemComponent();
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

	// 데미지 계산, 적용은 서버에서만 실행
	if (!SourceASC || !TargetASC || !TargetASC->GetOwner()->HasAuthority())
		return;
	
	// BaseDamage: SetByCaller에서 읽음 (없으면 0)
	float BaseDamage = Spec.GetSetByCallerMagnitude(DamageCallerTag, 0.f);
	if (BaseDamage <= 0.f)
	{
		return; // 줄 데미지 없음
	}
	
	const FGameplayTagContainer& DynTags = Spec.GetDynamicAssetTags();
	
	// Attack Type 판정
	const bool bIsBasic = DynTags.HasTag(UnitGameplayTags::Unit_AttackType_Basic);
	const bool bIsUltimate = DynTags.HasTag(UnitGameplayTags::Unit_AttackType_Ultimate);
	const bool bIsBonusDamage = DynTags.HasTag(UnitGameplayTags::Unit_AttackType_BonusDamage);

	// Damage Type 판정
	const bool bIsPhysical = DynTags.HasTag(UnitGameplayTags::Unit_DamageType_Physical);
	const bool bIsMagic = DynTags.HasTag(UnitGameplayTags::Unit_DamageType_Magic);
	const bool bIsTrueDamage = DynTags.HasTag(UnitGameplayTags::Unit_DamageType_TrueDamage);
	
	// Damage Flag 판정
	const bool bNoCrit = DynTags.HasTag(UnitGameplayTags::Unit_DamageFlag_NoCrit);
	const bool bNoVamp = DynTags.HasTag(UnitGameplayTags::Unit_DamageFlag_NoVamp);
	const bool bNoManaGain = DynTags.HasTag(UnitGameplayTags::Unit_DamageFlag_NoManaGain);
	const bool bNoSendHitEvent = DynTags.HasTag(UnitGameplayTags::Unit_DamageFlag_NoSendHitEvent);
	const bool bNoSendDamageAppliedEvent = DynTags.HasTag(UnitGameplayTags::Unit_DamageFlag_NoSendDamageAppliedEvent);

	FAggregatorEvaluateParameters EvalParams;
	EvalParams.SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	EvalParams.TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	// 캡처 값 가져오기
	auto GetMagnitude = [&ExecutionParams, &EvalParams](const FGameplayEffectAttributeCaptureDefinition& Def, float& OutVal)
	{
		OutVal = 0.f;
		ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(Def, EvalParams, OutVal);
	};

	// 기본 공격일 경우 회피율 계산
	if (bIsBasic)
	{
		float Evasion = 0.f;
		GetMagnitude(Captures.Evasion, Evasion);
		Evasion *= 0.01f;
		if (Evasion > 0.f && FMath::FRand() < Evasion)
		{
			// 데미지 입는 대상 회피 성공, 바로 리턴
			FGameplayCueParameters CueParams;
			CueParams.EffectContext = Spec.GetEffectContext();
			CueParams.Instigator = SourceASC->GetAvatarActor();
			CueParams.AggregatedSourceTags.AddTag(UnitGameplayTags::Unit_CombatText_Type_Miss);
	
			TargetASC->ExecuteGameplayCue(GameplayCueTags::GameplayCue_UI_Unit_CombatText, CueParams);
			
			return;
		}
	}
	
	// 공격 적중에 성공할 경우 이벤트 발생 (데미지 적용 전에 호출하는 이벤트)
	if (!bNoSendHitEvent)
	{
		FGameplayTag HitSucceedEventTag;
		if (bIsBasic)
		{
			HitSucceedEventTag = BasicHitSucceedEventTag;
		}
		else if (bIsUltimate)
		{
			HitSucceedEventTag = UltimateHitSucceedEventTag;
		}
		else if (bIsBonusDamage)
		{
			HitSucceedEventTag = BonusDmgHitSucceedEventTag;
		}
			
		if (HitSucceedEventTag.IsValid())
		{
			FGameplayEventData HitSucceedData;
			HitSucceedData.EventTag = HitSucceedEventTag;
			HitSucceedData.Instigator = SourceASC->GetAvatarActor();
			HitSucceedData.Target = TargetASC->GetAvatarActor();
			SourceASC->HandleGameplayEvent(HitSucceedEventTag, &HitSucceedData);
		}
		
		FGameplayEventData OnHitData;
		OnHitData.EventTag = OnHitEventTag;
		OnHitData.Instigator = SourceASC->GetAvatarActor();
		OnHitData.Target = TargetASC->GetAvatarActor();
		TargetASC->HandleGameplayEvent(OnHitEventTag, &OnHitData);
	}
	
	// 타입 데미지 배율
	float TypeMul = 0.f;
	if (bIsPhysical)
		GetMagnitude(Captures.PhysDamageMultiplier, TypeMul);
	else if (bIsMagic)
		GetMagnitude(Captures.MagicDamageMultiplier, TypeMul);
	TypeMul *= 0.01f;
	BaseDamage *= 1.f + TypeMul;
	
	bool bIsCritical = false;
	if (!bNoCrit)
	{
		// 치명타
		float CritChance = 0.f, CritMul = 30.f;
		GetMagnitude(Captures.CritChance, CritChance);
		CritChance *= 0.01f;
		GetMagnitude(Captures.CritMultiplier, CritMul);
		CritMul *= 0.01f;
	
		if (CritChance > 0.f && CritMul > 0.f)
		{
			if (FMath::FRand() < FMath::Clamp(CritChance, 0.f, 1.f))
			{
				bIsCritical = true;
				BaseDamage *= (1.f + CritMul);
			}
		}
	}

	float FinalDamage = FMath::Max(0.f, BaseDamage);

	if (!bIsTrueDamage)
	{
		float Defense = 0.f;
		if (bIsPhysical)
			GetMagnitude(Captures.PhysicalDefense, Defense);
		else if (bIsMagic)
			GetMagnitude(Captures.MagicDefense, Defense);
		
		// 데미지 경감 공식
		const float Mitigation = 100.f / (100.f + Defense);
		FinalDamage = FMath::Max(0.f, BaseDamage * Mitigation);

		float FlatBlock = 0.f;
		GetMagnitude(Captures.FlatDamageBlock, FlatBlock);
		FinalDamage = FMath::Max(0.f, FinalDamage - FMath::Max(0.f, FlatBlock));
	}

	if (FinalDamage <= KINDA_SMALL_NUMBER)
		return;

	// 최종 데미지 배율 적용
	float FinalMul = 0.f;
	GetMagnitude(Captures.DamageMultiplier, FinalMul);
	FinalMul *= 0.01f;
	FinalMul = FMath::Max(0.f, FinalMul);
	FinalDamage *= 1.f + FinalMul;
	
	// Health에 음수로 적용
	OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(
		UPCUnitAttributeSet::GetCurrentHealthAttribute(),
		EGameplayModOp::Additive,
		-FinalDamage));
	
	if (!bNoSendDamageAppliedEvent)
	{
		// 데미지 적용했다는 이벤트 발생
		FGameplayEventData EventData;
		EventData.EventTag = UnitGameplayTags::Unit_Event_DamageApplied;
		EventData.EventMagnitude = FinalDamage;
		EventData.Instigator = SourceASC->GetAvatarActor();
		EventData.Target = TargetASC->GetAvatarActor();
	
		SourceASC->HandleGameplayEvent(EventData.EventTag, &EventData);
	}
	
	const bool bIsHeroTarget = (TargetASC && TargetASC->GetAttributeSet(UPCHeroUnitAttributeSet::StaticClass()) != nullptr);
	if (bIsHeroTarget)
	{
		// 감소전 피해량 & 감소후 피해량에 따라 마나 회복 (영웅 전용)
		if (!bNoManaGain)
		{
			float ManaGain = BaseDamage * 0.01f + FinalDamage * 0.07f;
			ManaGain = FMath::Clamp(ManaGain, 0.f, 50.f);
		
			OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(
				UPCHeroUnitAttributeSet::GetCurrentManaAttribute(),
				EGameplayModOp::Additive,
				ManaGain));
		}
	}
	
	// 공격자 피흡 적용 (영웅 전용)
	if (!bNoVamp)
	{
		float VampPct = 0.f;
		GetMagnitude(Captures.LifeSteal, VampPct);
		
		VampPct *= 0.01f;
	
		if (VampPct > 0.f)
		{
			const float HealAmount = FinalDamage * VampPct;
			if (HealAmount > KINDA_SMALL_NUMBER)
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
	
	// 최종 데미지 Combat Text UI GameplayCue 실행
	FGameplayTag CombatTextTypeTag;
	if (bIsTrueDamage)
		CombatTextTypeTag = UnitGameplayTags::Unit_CombatText_Type_Damage_TrueDamage;
	else if (bIsMagic)
		CombatTextTypeTag = UnitGameplayTags::Unit_CombatText_Type_Damage_Magic;
	else
		CombatTextTypeTag = UnitGameplayTags::Unit_CombatText_Type_Damage_Physical;

	if (CombatTextTypeTag.IsValid())
	{
		FGameplayCueParameters CueParams;
		CueParams.EffectContext = Spec.GetEffectContext();
		CueParams.RawMagnitude = FinalDamage;
		CueParams.Instigator = SourceASC->GetAvatarActor();
		CueParams.AggregatedSourceTags.AddTag(CombatTextTypeTag);

		if (bIsCritical)
			CueParams.AggregatedSourceTags.AddTag(UnitGameplayTags::Unit_CombatText_Flag_Critical);
	
		TargetASC->ExecuteGameplayCue(GameplayCueTags::GameplayCue_UI_Unit_CombatText, CueParams);
	}
}

const UGameplayEffect* UPCUnitDamageExec::ResolveHealGE(const UWorld* World) const
{
	if (!World)
		return nullptr;

	if (UPCUnitGERegistrySubsystem* GERegistrySubsystem = World->GetSubsystem<UPCUnitGERegistrySubsystem>())
		return GERegistrySubsystem->GetGE_CDO(HealGEClassTag);
	
	return nullptr;
}
