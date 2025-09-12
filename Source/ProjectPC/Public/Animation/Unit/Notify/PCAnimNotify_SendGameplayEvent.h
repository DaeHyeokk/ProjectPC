// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "PCAnimNotify_SendGameplayEvent.generated.h"

/**
 * 
 */
UCLASS(meta=(DisplayName="Send Gameplay Event To Owner"))
class PROJECTPC_API UPCAnimNotify_SendGameplayEvent : public UAnimNotify
{
	GENERATED_BODY()
	
public:
	virtual void Notify(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Gameplay Event")
	FGameplayTag EventTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Gameplay Event")
	bool bServerOnly = true;
};
