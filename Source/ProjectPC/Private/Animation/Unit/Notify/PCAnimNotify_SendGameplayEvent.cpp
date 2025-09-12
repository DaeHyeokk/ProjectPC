// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/Unit/Notify/PCAnimNotify_SendGameplayEvent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/GameplayAbilityTypes.h"


void UPCAnimNotify_SendGameplayEvent::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                             const FAnimNotifyEventReference& EventReference)
{
	if (!MeshComp)
		return;
	AActor* Owner = MeshComp->GetOwner();
	if (!Owner || !EventTag.IsValid())
		return;

	if (bServerOnly && !Owner->HasAuthority())
		return;
	
	FGameplayEventData Payload;
	Payload.EventTag       = EventTag;
	Payload.Instigator     = Owner;
	Payload.Target         = Owner;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Owner, EventTag, Payload);
}
	
