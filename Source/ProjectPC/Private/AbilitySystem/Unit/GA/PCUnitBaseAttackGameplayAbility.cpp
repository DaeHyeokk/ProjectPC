// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Unit/GA/PCUnitBaseAttackGameplayAbility.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Animation/Unit/Notify/PCAnimNotify_SpawnProjectile.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/Projectile/PCBaseProjectile.h"
#include "Character/Unit/PCBaseUnitCharacter.h"
#include "DataAsset/Unit/PCDataAsset_UnitAnimSet.h"

UPCUnitBaseAttackGameplayAbility::UPCUnitBaseAttackGameplayAbility()
{
	AbilityTags.AddTag(UnitGameplayTags::Unit_Ability_Attack);

	ActivationBlockedTags.AddTag(UnitGameplayTags::Unit_State_Combat_Attacking);

	BlockAbilitiesWithTag.AddTag(UnitGameplayTags::Unit_Ability_Attack);
	
	ActivationOwnedTags.AddTag(UnitGameplayTags::Unit_State_Combat_Attacking);
}

void UPCUnitBaseAttackGameplayAbility::OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilitySpec& Spec)
{
	Super::OnAvatarSet(ActorInfo, Spec);
	
	if (Unit)
		SetMontageConfig();
}

void UPCUnitBaseAttackGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                       const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                                       const FGameplayEventData* TriggerEventData)
{
	if (!HasAuthority(&ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
		return;
	}
		
	if (UAnimMontage* Montage = MontageConfig.Montage)
	{
		CurrentTarget = nullptr;
		SetCurrentTarget(GetAvatarActorFromActorInfo());
		if (!CurrentTarget.IsValid())
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, false, true);
			return;
		}
			
		StartPlayMontageAndWaitTask(Montage, false);
		
		if (AbilityConfig.bSpawnProjectile)
		{
			StartProjectileSpawnSucceedWaitTask();
		}
		else
		{
			StartAttackSucceedWaitTask();
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
	true
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
		true
		);
	if (WaitEventTask)
	{
		WaitEventTask->EventReceived.AddDynamic(this, &ThisClass::OnSpawnProjectileSucceed);
		WaitEventTask->ReadyForActivation();
	}
}

void UPCUnitBaseAttackGameplayAbility::OnAttackSucceed(FGameplayEventData Payload)
{
	AActor* Avatar = GetAvatarActorFromActorInfo();
	UAbilitySystemComponent* ASC = Unit->GetAbilitySystemComponent();
	if (!Avatar || !ASC)
		return;
	
	if (CurrentTarget.IsValid())
	{
		ApplyReceivedEventEffectSpec(ASC, AttackSucceedTag, CurrentTarget.Get());
	}

	AttackCommit();
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
			}
		}
	}
	
	AttackCommit();
}

//공격이 완료 되었을 때 호출 (원거리: 발사체 생성, 근거리: Hit 타이밍)
void UPCUnitBaseAttackGameplayAbility::AttackCommit()
{
	if (!CommitAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo))
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
		return;
	}
	
	// 공격 완료 즉시 적용하는 GE Apply
	if (UAbilitySystemComponent* ASC = Unit->GetAbilitySystemComponent())
	{
		ApplyCommittedEffectSpec(ASC, CurrentTarget.Get());
	}
	
	// 후딜 없으면 바로 어빌리티 종료
	if (!MontageConfig.bHasRecovery)
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
}

