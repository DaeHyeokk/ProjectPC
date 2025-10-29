// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Unit/GA/PCUnitMontagePlayGameplayAbility.h"

#include "BaseGameplayTags.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Character/Unit/PCBaseUnitCharacter.h"


UPCUnitMontagePlayGameplayAbility::UPCUnitMontagePlayGameplayAbility()
{
	ActivationBlockedTags.AddTag(UnitGameplayTags::Unit_State_Combat_Stun);
	
	CancelAbilitiesWithTag.AddTag(UnitGameplayTags::Unit_State_Combat_Stun);
	CancelAbilitiesWithTag.AddTag(UnitGameplayTags::Unit_State_Combat_Dead);
}

void UPCUnitMontagePlayGameplayAbility::OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo,
                                                    const FGameplayAbilitySpec& Spec)
{
	Super::OnAvatarSet(ActorInfo, Spec);
	
	if (Unit)
		SetMontage();
}

void UPCUnitMontagePlayGameplayAbility::SetMontage()
{
	if (const UPCDataAsset_UnitAnimSet* UnitAnimSet = Unit ? Unit->GetUnitAnimSetDataAsset() : nullptr)
	{
		const FGameplayTag MontageTag = GetMontageTag();
		Montage = UnitAnimSet->GetMontageByTag(MontageTag);
	}
}

void UPCUnitMontagePlayGameplayAbility::StartPlayMontageAndWaitTask(const bool bStopWhenAbilityEnds)
{
	if (!Montage)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, true);
		return;
	}
	
	const float MontagePlayRate = GetMontagePlayRate();
			
	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this, NAME_None, Montage, MontagePlayRate, NAME_None, bStopWhenAbilityEnds);
	
	MontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageFinished);
	MontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageFinished);
	MontageTask->OnBlendOut.AddDynamic(this, &ThisClass::OnMontageFinished);
	MontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageFinished);
	MontageTask->ReadyForActivation();
}

void UPCUnitMontagePlayGameplayAbility::OnMontageFinished()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
}


// ==== 디버깅용 ====
// void UPCUnitMontagePlayGameplayAbility::OnMontageCompleted()
// {
// 	UE_LOG(LogTemp, Warning, TEXT("Montage Completed"));
// 	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
// }
//
// void UPCUnitMontagePlayGameplayAbility::OnMontageCancelled()
// {
// 	UE_LOG(LogTemp, Warning, TEXT("Montage Cancelled"));
// 	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, true);
// }
//
// void UPCUnitMontagePlayGameplayAbility::OnMontageBlendOut()
// {
// 	UE_LOG(LogTemp, Warning, TEXT("Montage BlendOut"));
// 	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
// }
//
// void UPCUnitMontagePlayGameplayAbility::OnMontageInterrupted()
// {
// 	UE_LOG(LogTemp, Warning, TEXT("Montage Interrupted"));
// 	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
// }