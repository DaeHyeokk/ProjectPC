// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Unit/GA/PCUnitBaseAttackGameplayAbility.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Animation/Unit/Notify/PCAnimNotify_SpawnProjectile.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/Projectile/PCBaseProjectile.h"
#include "Character/Unit/PCBaseUnitCharacter.h"
#include "DataAsset/Projectile/PCDataAsset_ProjectileData.h"
#include "DataAsset/Unit/PCDataAsset_UnitAnimSet.h"
#include "GameFramework/WorldSubsystem/PCProjectilePoolSubsystem.h"
#include "Kismet/KismetMathLibrary.h"

UPCUnitBaseAttackGameplayAbility::UPCUnitBaseAttackGameplayAbility()
{
	AbilityTags.AddTag(UnitGameplayTags::Unit_Action_Attack);

	ActivationBlockedTags.AddTag(UnitGameplayTags::Unit_State_Combat_Attacking);

	BlockAbilitiesWithTag.AddTag(UnitGameplayTags::Unit_Action_Attack);
	
	ActivationOwnedTags.AddTag(UnitGameplayTags::Unit_State_Combat_Attacking);
}

void UPCUnitBaseAttackGameplayAbility::OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilitySpec& Spec)
{
	Super::OnAvatarSet(ActorInfo, Spec);
	
	if (Unit)
		SetMontageConfig(ActorInfo);
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
			
		StartPlayMontageAndWaitTask(Montage);
		
		if (AbilityConfig.bSpawnProjectile)
		{
			StartProjectileSpawnSucceedWaitTask();
		}
		else
		{
			StartHitSucceedWaitTask();
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

void UPCUnitBaseAttackGameplayAbility::SetMontageConfig(const FGameplayAbilityActorInfo* ActorInfo)
{
	if (const UPCDataAsset_UnitAnimSet* UnitAnimSet = Unit ? Unit->GetUnitAnimSetDataAsset() : nullptr)
	{
		const FGameplayTag MontageTag = GetMontageTag();
		MontageConfig = UnitAnimSet->GetMontageConfigByTag(MontageTag);
	}
}

void UPCUnitBaseAttackGameplayAbility::StartHitSucceedWaitTask()
{
	UAbilityTask_WaitGameplayEvent* WaitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
	this,
	HitSucceedTag,
	nullptr,
	true
	);
	if (WaitEventTask)
	{
		WaitEventTask->EventReceived.AddDynamic(this, &ThisClass::OnHitSucceed);
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

void UPCUnitBaseAttackGameplayAbility::StartPlayMontageAndWaitTask(UAnimMontage* Montage)
{
	const float MontagePlayRate = GetMontagePlayRate(Montage);
			
	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this, NAME_None, Montage, MontagePlayRate, NAME_None, false);
	
	MontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageFinished);
	MontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageFinished);
	MontageTask->OnBlendOut.AddDynamic(this, &ThisClass::OnMontageFinished);
	MontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageFinished);
	MontageTask->ReadyForActivation();
}

void UPCUnitBaseAttackGameplayAbility::OnHitSucceed(FGameplayEventData Payload)
{
	AActor* Avatar = GetAvatarActorFromActorInfo();
	UAbilitySystemComponent* ASC = Unit->GetAbilitySystemComponent();
	if (!Avatar || !ASC)
		return;
	
	if (CurrentTarget.IsValid())
	{
		if (const FPCEffectSpecList* List = AbilityConfig.OnReceivedEventEffectsMap.Find(HitSucceedTag))
		{
			for (auto& Spec : List->EffectSpecs)
			{
				if (Spec)
					Spec->ApplyEffect(ASC, CurrentTarget.Get());
			}
		}
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
	
	// 공격 완료 즉시 자신에게 적용하는 GE Apply
	if (UAbilitySystemComponent* ASC = Unit->GetAbilitySystemComponent())
	{
		const FPCEffectSpecList& List = AbilityConfig.OnCommittedEffectSpecs;
		if (List.EffectSpecs.Num() > 0)
		{
			for (const auto& EffectSpec : List.EffectSpecs)
			{
				if (EffectSpec)
					EffectSpec->ApplyEffect(ASC, GetAvatarActorFromActorInfo());
			}
		}
	}
	
	// 후딜 없으면 바로 어빌리티 종료
	if (!MontageConfig.bHasRecovery)
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
}

void UPCUnitBaseAttackGameplayAbility::OnMontageFinished()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
}

// ==== 디버깅용 ====
// void UPCUnitBaseAttackGameplayAbility::OnMontageCompleted()
// {
// 	UE_LOG(LogTemp, Warning, TEXT("Montage Completed"));
// 	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
// }
//
// void UPCUnitBaseAttackGameplayAbility::OnMontageCancelled()
// {
// 	UE_LOG(LogTemp, Warning, TEXT("Montage Cancelled"));
// 	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, true);
// }
//
// void UPCUnitBaseAttackGameplayAbility::OnMontageBlendOut()
// {
// 	UE_LOG(LogTemp, Warning, TEXT("Montage BlendOut"));
// 	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
// }
//
// void UPCUnitBaseAttackGameplayAbility::OnMontageInterrupted()
// {
// 	UE_LOG(LogTemp, Warning, TEXT("Montage Interrupted"));
// 	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
// }
