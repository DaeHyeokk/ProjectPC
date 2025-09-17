// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Unit/GA/PCUnitBaseAttackGameplayAbility.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystem/Unit/AttributeSet/PCUnitAttributeSet.h"
#include "Character/Unit/PCBaseUnitCharacter.h"
#include "DataAsset/Unit/PCDataAsset_UnitAnimSet.h"

UPCUnitBaseAttackGameplayAbility::UPCUnitBaseAttackGameplayAbility()
{
	AbilityTags.AddTag(UnitGameplayTags::Unit_Action_Attack);

	BlockAbilitiesWithTag.AddTag(UnitGameplayTags::Unit_Action_Attack);

	ActivationBlockedTags.AddTag(UnitGameplayTags::Unit_State_Combat_Attacking);
	
	ActivationOwnedTags.AddTag(UnitGameplayTags::Unit_State_Combat_Attacking);
}

void UPCUnitBaseAttackGameplayAbility::OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilitySpec& Spec)
{
	Super::OnAvatarSet(ActorInfo, Spec);
	
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	UnitAttrSet = ASC ? ASC->GetSet<UPCUnitAttributeSet>() : nullptr;

	const APCBaseUnitCharacter* UnitCharacter = Cast<APCBaseUnitCharacter>(ActorInfo->OwnerActor.Get());
	UnitAnimSet = UnitCharacter ? UnitCharacter->GetUnitAnimSetDataAsset() : nullptr;

	if (UnitAnimSet)
		SetMontageConfig(ActorInfo);
}

void UPCUnitBaseAttackGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                       const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                                       const FGameplayEventData* TriggerEventData)
{
	if (HasAuthority(&ActivationInfo))
	{
		if (UAnimMontage* Montage = AttackMontageConfig.Montage)
		{
			StartPlayMontageAndWaitTask(Montage);

			if (AttackMontageConfig.bHasWindup)
			{
				StartAttackCommitWaitTask();
			}
			else
			{
				// Windup 동작이 없으면 바로 커밋
				if (CommitAbility(Handle, ActorInfo, ActivationInfo))
					ApplyGameplayEffect();
			}

			// 후딜 없으면 바로 어빌리티 종료
			if (!AttackMontageConfig.bHasRecovery)
				EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Unit Ability Montage Nullptr"));
			EndAbility(Handle, ActorInfo, ActivationInfo, false, true);
		}
	}
}

void UPCUnitBaseAttackGameplayAbility::StartAttackCommitWaitTask()
{
	const FGameplayTag AttackCommitEventTag = UnitGameplayTags::Unit_Event_AttackCommit;
	UAbilityTask_WaitGameplayEvent* WaitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		AttackCommitEventTag,
		nullptr,
		true
		);
	if (WaitEventTask)
	{
		WaitEventTask->EventReceived.AddDynamic(this, &ThisClass::OnAttackCommit);
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

//공격이 완료 되었을 때 호출 (원거리: 발사체 생성, 근거리: Hit 타이밍)
void UPCUnitBaseAttackGameplayAbility::OnAttackCommit(FGameplayEventData Payload)
{
	if (CommitAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo))
		ApplyGameplayEffect();

	if (!AttackMontageConfig.bHasRecovery)
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
