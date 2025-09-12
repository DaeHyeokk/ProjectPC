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
	
	ActivationBlockedTags.AddTag(UnitGameplayTags::Unit_Action_Attack);

	BlockAbilitiesWithTag.AddTag(UnitGameplayTags::Unit_Action_Attack);

	ActivationOwnedTags.AddTag(UnitGameplayTags::Unit_Action_Attack);
}

void UPCUnitBaseAttackGameplayAbility::OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilitySpec& Spec)
{
	Super::OnAvatarSet(ActorInfo, Spec);
	
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	UnitAttrSet = ASC ? ASC->GetSet<UPCUnitAttributeSet>() : nullptr;

	const APCBaseUnitCharacter* UnitCharacter = Cast<APCBaseUnitCharacter>(ActorInfo->OwnerActor.Get());
	UnitAnimSet = UnitCharacter ? UnitCharacter->GetUnitAnimSetDataAsset() : nullptr;
}

void UPCUnitBaseAttackGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                       const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                                       const FGameplayEventData* TriggerEventData)
{
	if (HasAuthority(&ActivationInfo))
	{
		if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, false, true);
		}
		
		if (UAnimMontage* Montage = GetMontage(ActorInfo))
		{
			UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
				this, NAME_None, Montage, 1.f, NAME_None, false);
			UE_LOG(LogTemp, Warning, TEXT("Call Montage PlayAndWait!"));
			MontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageCompleted);
			MontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageInterrupted);
			MontageTask->OnBlendOut.AddDynamic(this, &ThisClass::OnMontageBlendOut);
			MontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageCancelled);
			MontageTask->ReadyForActivation();

			ApplyGameplayEffect();
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Unit Ability Montage Nullptr"));
			EndAbility(Handle, ActorInfo, ActivationInfo, false, true);
		}
	}
}

// 공격이 완료 되었을 때 호출 (원거리: 발사체 생성, 근거리: Hit AnimNotify 호출)
// void UPCUnitBaseAttackGameplayAbility::OnAttackCommit(FGameplayEventData Payload)
// {
// 	CommitAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo);
// 	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
// }

void UPCUnitBaseAttackGameplayAbility::OnMontageCompleted()
{
	UE_LOG(LogTemp, Warning, TEXT("Montage Completed"));
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
}

void UPCUnitBaseAttackGameplayAbility::OnMontageCancelled()
{
	UE_LOG(LogTemp, Warning, TEXT("Montage Cancelled"));
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, true);
}

void UPCUnitBaseAttackGameplayAbility::OnMontageBlendOut()
{
	UE_LOG(LogTemp, Warning, TEXT("Montage BlendOut"));
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
}

void UPCUnitBaseAttackGameplayAbility::OnMontageInterrupted()
{
	UE_LOG(LogTemp, Warning, TEXT("Montage Interrupted"));
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
}
