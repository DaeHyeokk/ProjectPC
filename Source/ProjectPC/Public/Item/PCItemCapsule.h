// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"
#include "PCItemCapsule.generated.h"

class APCPlayerState;

UCLASS()
class PROJECTPC_API APCItemCapsule : public AActor
{
	GENERATED_BODY()
	
public:	
	APCItemCapsule();

	void SetOwnerPlayer(int32 NewTeamIndex);

private:
	int32 TeamIndex = -1;
	FGameplayTag RewardTag;
	
	void SetRandomRewardTag();
	void AddRewardToPlayer(APCPlayerState* TargetPlayer);

protected:
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
};
