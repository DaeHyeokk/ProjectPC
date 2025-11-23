// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Unit/GA/Synergy/PCSynergyGuardianGameplayAbility.h"

#include "AbilitySystemComponent.h"
#include "BaseGameplayTags.h"
#include "Character/Unit/PCBaseUnitCharacter.h"
#include "Utility/PCUnitCombatUtils.h"
#include "Particles/ParticleSystem.h"

UPCSynergyGuardianGameplayAbility::UPCSynergyGuardianGameplayAbility()
{
	AbilityTags.AddTag(UnitGameplayTags::Unit_Ability_Synergy_Guardian);
}

void UPCSynergyGuardianGameplayAbility::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);

	// 능력이 부여되자마자 발동
	if (ActorInfo && ActorInfo->IsNetAuthority())
	{
		ActorInfo->AbilitySystemComponent->TryActivateAbility(Spec.Handle);
	}
}

void UPCSynergyGuardianGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!HasAuthority(&ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
		return;
	}
	
	TArray<APCBaseUnitCharacter*> Allies;
	FindNearlyAllies(Allies);
	
	// 수호자가 아닌 아군 방어력 증가
	for (APCBaseUnitCharacter* Ally : Allies)
	{
		UAbilitySystemComponent* AllyASC = Ally->GetAbilitySystemComponent();
		TArray<FActiveGameplayEffectHandle> EffectHandles = ApplyActivatedEffectSpec(AllyASC, Ally);
		if (!EffectHandles.IsEmpty())
		{
			ActiveEffectsMap.Add(AllyASC, EffectHandles);
		}

		PlayParticle(Ally, NearUnitParticle);
	}

	PlayParticle(Unit, AbilityActiveParticle);
}

void UPCSynergyGuardianGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	RevokeAllAppliedEffects();
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UPCSynergyGuardianGameplayAbility::RevokeAllAppliedEffects()
{
	for (auto& KV : ActiveEffectsMap)
	{
		if (UAbilitySystemComponent* ASC = KV.Key.Get())
		{
			for (FActiveGameplayEffectHandle& Handle : KV.Value)
			{
				if (Handle.IsValid())
				{
					ASC->RemoveActiveGameplayEffect(Handle);
				}
			}
		}
	}
	ActiveEffectsMap.Reset();
}

void UPCSynergyGuardianGameplayAbility::FindNearlyAllies(TArray<APCBaseUnitCharacter*>& OutAllies) const
{
	OutAllies.Reset();
	
	if (APCBaseUnitCharacter* Owner = Cast<APCBaseUnitCharacter>(GetAvatarActorFromActorInfo()))
	{
		if (APCCombatBoard* Board = Owner->GetOnCombatBoard())
		{
			const FIntPoint OwnerPoint = Board->GetFieldUnitPoint(Owner);
			if (OwnerPoint == FIntPoint::NoneValue)
				return;

			for (const FIntPoint& Dir : PCUnitCombatUtils::GetDirections(OwnerPoint.Y % 2 == 0))
			{
				FIntPoint NearlyPoint = OwnerPoint + Dir;

				// 이동할 좌표가 유효한 좌표일경우 탐색
				if (Board->IsInRange(NearlyPoint.Y, NearlyPoint.X))
				{
					APCBaseUnitCharacter* NearlyUnit = Board->GetUnitAt(NearlyPoint.Y, NearlyPoint.X);
					// 근처에 있는 유닛이 아군일 경우
					if (NearlyUnit && !PCUnitCombatUtils::IsHostile(Owner, NearlyUnit))
					{
						if (const UAbilitySystemComponent* NearlyUnitASC = NearlyUnit->GetAbilitySystemComponent())
						{
							// 근처에 있는 유닛이 수호자가 아닐경우 추가
							if (!NearlyUnitASC->HasMatchingGameplayTag(GuardianSynergyTag))
							{
								OutAllies.Add(NearlyUnit);
							}
						}
					}
				}
			}
		}
	}
}

void UPCSynergyGuardianGameplayAbility::PlayParticle(const APCBaseUnitCharacter* TargetUnit, UParticleSystem* Particle) const
{
	if (!HasAuthority(&CurrentActivationInfo) || !Particle || !TargetUnit)
		return;

	UAbilitySystemComponent* ASC = TargetUnit->GetAbilitySystemComponent();
	USkeletalMeshComponent* UnitMesh = TargetUnit->GetMesh();
	
	if (ASC && UnitMesh)
	{
		FGameplayCueParameters Params;
		Params.TargetAttachComponent = UnitMesh;
		Params.SourceObject = Particle;

		FGameplayEffectContextHandle Ctx = ASC->MakeEffectContext();
		FHitResult HitResult;
		HitResult.BoneName = AttachedSocketName;
		Ctx.AddHitResult(HitResult);
		Params.EffectContext = Ctx;
		
		ASC->ExecuteGameplayCue(GameplayCueTags::GameplayCue_VFX_Unit_PlayEffectAtSocket, Params);
	}
}
