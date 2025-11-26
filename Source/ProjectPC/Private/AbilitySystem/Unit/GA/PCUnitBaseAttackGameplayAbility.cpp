// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Unit/GA/PCUnitBaseAttackGameplayAbility.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Animation/Unit/Notify/PCAnimNotify_SendGameplayEvent.h"
#include "Animation/Unit/Notify/PCAnimNotify_SpawnProjectile.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/Projectile/PCBaseProjectile.h"
#include "Character/Unit/PCBaseUnitCharacter.h"

UPCUnitBaseAttackGameplayAbility::UPCUnitBaseAttackGameplayAbility()
{
	AbilityTags.AddTag(UnitGameplayTags::Unit_Ability_Attack);

	ActivationBlockedTags.AddTag(UnitGameplayTags::Unit_State_Combat_Attacking);
	ActivationBlockedTags.AddTag(UnitGameplayTags::Unit_State_Combat_Jumping);

	BlockAbilitiesWithTag.AddTag(UnitGameplayTags::Unit_Ability_Attack);
	BlockAbilitiesWithTag.AddTag(UnitGameplayTags::Unit_Ability_Movement_Jump);
	
	ActivationOwnedTags.AddTag(UnitGameplayTags::Unit_State_Combat_Attacking);
}

void UPCUnitBaseAttackGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                       const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                                       const FGameplayEventData* TriggerEventData)
{
	// 서버가 아니거나 Commit에 실패했을 경우 어빌리티 종료
	if (!HasAuthority(&ActivationInfo) || !CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
		return;
	}
	
	if (Montage)
	{
		CurrentTarget = nullptr;
		SetCurrentTarget(GetAvatarActorFromActorInfo());
		if (!CurrentTarget.IsValid())
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, false, true);
			return;
		}
		
		if (AbilityConfig.bSpawnProjectile)
		{
			StartProjectileSpawnSucceedWaitTask();
		}
		else
		{
			StartAttackSucceedWaitTask();
		}

		StartPlayMontageAndWaitTask();
		
		// 공격 완료 즉시 적용하는 GE Apply
		if (UAbilitySystemComponent* ASC = Unit->GetAbilitySystemComponent())
		{
			ApplyCommittedEffectSpec(ASC, CurrentTarget.Get());
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Unit Ability Montage Nullptr"));
		EndAbility(Handle, ActorInfo, ActivationInfo, false, true);
	}
}

void UPCUnitBaseAttackGameplayAbility::SetCurrentTarget(const AActor* Avatar)
{
	if (!Avatar)
		return;
	
	if (const AController* Con = Cast<AController>(Avatar->GetInstigatorController()))
	{
		if (const UBlackboardComponent* BB = Con->FindComponentByClass<UBlackboardComponent>())
		{
			CurrentTarget = Cast<AActor>(BB->GetValueAsObject(TEXT("TargetUnit")));
		}
	}
}

void UPCUnitBaseAttackGameplayAbility::StartAttackSucceedWaitTask()
{
	UAbilityTask_WaitGameplayEvent* WaitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		AttackSucceedTag,
		nullptr,
		false
	);
	if (WaitEventTask)
	{
		WaitEventTask->EventReceived.AddDynamic(this, &ThisClass::OnAttackSucceed);
		WaitEventTask->ReadyForActivation();
	}
}

void UPCUnitBaseAttackGameplayAbility::StartProjectileSpawnSucceedWaitTask()
{
	UAbilityTask_WaitGameplayEvent* WaitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		SpawnProjectileSucceedTag,
		nullptr,
		false
		);
	if (WaitEventTask)
	{
		WaitEventTask->EventReceived.AddDynamic(this, &ThisClass::OnSpawnProjectileSucceed);
		WaitEventTask->ReadyForActivation();
	}
}

void UPCUnitBaseAttackGameplayAbility::OnAttackSucceed(FGameplayEventData Payload)
{
	UAbilitySystemComponent* ASC = Unit ? Unit->GetAbilitySystemComponent() : nullptr;
	if (!ASC)
		return;
	
	if (CurrentTarget.IsValid())
	{
		ApplyReceivedEventEffectSpec(ASC, AttackSucceedTag, CurrentTarget.Get());
		PlayAttackSound();
	}
}

void UPCUnitBaseAttackGameplayAbility::OnSpawnProjectileSucceed(FGameplayEventData Payload)
{
	for (const TSharedPtr<FGameplayAbilityTargetData>& TD : Payload.TargetData.Data)
	{
		if (const FTargetData_Projectile* P = static_cast<FTargetData_Projectile*>(TD.Get()))
		{
			if (APCBaseProjectile* Projectile = P->Projectile.Get())
			{
				Projectile->SetEffectSpecs(AbilityConfig.ProjectilePayloadEffectSpecs.EffectSpecs);
				PlayAttackSound();
			}
		}
	}
}

void UPCUnitBaseAttackGameplayAbility::PlayAttackSound() const
{
	UAbilitySystemComponent* ASC = Unit ? Unit->GetAbilitySystemComponent() : nullptr;
	if (!ASC)
		return;

	ASC->ExecuteGameplayCue(GameplayCueTags::GameplayCue_SFX_Unit_AttackSound);
}
