// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Task/BTTask_TryJumpAbility.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "AIController.h"
#include "BaseGameplayTags.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"


UBTTask_TryJumpAbility::UBTTask_TryJumpAbility()
{
	NodeName = TEXT("Try Activate Jump Ability");
	bNotifyTaskFinished = true;
	bNotifyTick = false;
}

EBTNodeResult::Type UBTTask_TryJumpAbility::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB)
		return EBTNodeResult::Failed;
	
	AAIController* AIC = OwnerComp.GetAIOwner();
	APawn* Pawn = AIC ? AIC->GetPawn() : nullptr;
	
	if (!Pawn)
	{
		BB->ClearValue(JumpLocationKey.SelectedKeyName);
		return EBTNodeResult::Failed;
	}

	UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Pawn);
	if (!ASC)
	{
		BB->ClearValue(JumpLocationKey.SelectedKeyName);
		return EBTNodeResult::Failed;
	}
	
	FVector JumpLocation = BB->GetValueAsVector(JumpLocationKey.SelectedKeyName);

	FGameplayAbilityTargetingLocationInfo SourceLoc;
	SourceLoc.LocationType = EGameplayAbilityTargetingLocationType::ActorTransform;
	SourceLoc.SourceActor = Pawn;
	
	FGameplayAbilityTargetingLocationInfo TargetLoc;
	TargetLoc.LocationType = EGameplayAbilityTargetingLocationType::LiteralTransform;
	TargetLoc.LiteralTransform = FTransform(JumpLocation);

	FGameplayEventData EventData;
	EventData.EventTag = UnitGameplayTags::Unit_Event_Jump_Start;
	EventData.TargetData = UAbilitySystemBlueprintLibrary::AbilityTargetDataFromLocations(SourceLoc, TargetLoc);

	ASC->HandleGameplayEvent(EventData.EventTag, &EventData);

	BB->ClearValue(JumpLocationKey.SelectedKeyName);
	return EBTNodeResult::Succeeded;
}