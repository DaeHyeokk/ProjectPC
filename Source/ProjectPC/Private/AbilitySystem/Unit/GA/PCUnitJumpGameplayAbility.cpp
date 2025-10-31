// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Unit/GA/PCUnitJumpGameplayAbility.h"

#include "BaseGameplayTags.h"
#include "Abilities/Tasks/AbilityTask_ApplyRootMotionJumpForce.h"
#include "Character/Unit/PCBaseUnitCharacter.h"
#include "Controller/Unit/PCUnitAIController.h"
#include "GameFramework/RootMotionSource.h"


UPCUnitJumpGameplayAbility::UPCUnitJumpGameplayAbility()
{
	AbilityTags.AddTag(UnitGameplayTags::Unit_Ability_Movement_Jump);
	
	ActivationBlockedTags.AddTag(UnitGameplayTags::Unit_State_Combat_Attacking);
	ActivationBlockedTags.AddTag(UnitGameplayTags::Unit_State_Combat_Jumping);

	BlockAbilitiesWithTag.AddTag(UnitGameplayTags::Unit_Ability_Attack);
	BlockAbilitiesWithTag.AddTag(UnitGameplayTags::Unit_Ability_Movement_Jump);
	
	ActivationOwnedTags.AddTag(UnitGameplayTags::Unit_State_Combat_Jumping);
	
	FAbilityTriggerData TriggerData;;
	TriggerData.TriggerTag = UnitGameplayTags::Unit_Event_Jump_Start;
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(TriggerData);
}

void UPCUnitJumpGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!HasAuthority(&ActivationInfo) || !Unit || !TriggerEventData)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
		return;
	}

	APCUnitAIController* AIC = Unit ? Cast<APCUnitAIController>(Unit->GetController()) : nullptr;
	if (!AIC)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
		return;
	}
	
	const FGameplayAbilityTargetData* TD = TriggerEventData->TargetData.Get(0);
	if (!TD)
	{
		AIC->OnJumpCompleted(false);
		EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
		return;
	}

	const FVector StartLoc = TD->GetOrigin().GetLocation();
	const FVector TargetLoc = TD->GetEndPoint();
	
	const FVector ToTarget = TargetLoc - StartLoc;
	const FVector ToTargetXY = FVector(ToTarget.X, ToTarget.Y, 0.f);
	const float HorizontalDist = ToTargetXY.Size();

	FRotator JumpRotation;
	if (HorizontalDist > KINDA_SMALL_NUMBER)
	{
		JumpRotation = ToTargetXY.GetSafeNormal().Rotation();
	}
	else
	{
		JumpRotation = ActorInfo->AvatarActor->GetActorForwardVector().GetSafeNormal2D().Rotation();
	}
	
	const float Height = 500.f;
	const float Duration = 1.f;
	const float MinimumLandedTriggerTime = 0.05f;
	const bool bFinishOnLanded = true;
	const ERootMotionFinishVelocityMode FinishVelMode = ERootMotionFinishVelocityMode::MaintainLastRootMotionVelocity;
	const FVector SetVelOnFinish = FVector::ZeroVector;
	
	auto* Task = UAbilityTask_ApplyRootMotionJumpForce::ApplyRootMotionJumpForce(
		this, NAME_None,
		JumpRotation, HorizontalDist, Height, Duration,
		MinimumLandedTriggerTime,
		bFinishOnLanded, FinishVelMode, SetVelOnFinish,
		0.f, nullptr, nullptr);

	Task->OnLanded.AddDynamic(this, &ThisClass::OnLanded);
	Task->ReadyForActivation();
}

void UPCUnitJumpGameplayAbility::OnLanded()
{
	if (Unit)
	{
		if (APCUnitAIController* AIC = Cast<APCUnitAIController>(Unit->GetController()))
		{
			AIC->OnJumpCompleted(true);
		}
	}
	
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
}
