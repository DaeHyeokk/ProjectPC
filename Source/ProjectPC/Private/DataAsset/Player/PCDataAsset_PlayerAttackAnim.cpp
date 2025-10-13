// Fill out your copyright notice in the Description page of Project Settings.


#include "DataAsset/Player/PCDataAsset_PlayerAttackAnim.h"


TSoftObjectPtr<UAnimMontage> UPCDataAsset_PlayerAttackAnim::GetAttackMontage(FGameplayTag CharacterTag, FGameplayTag AttackTypeTag)
{
	FAnimationFindKey Key(CharacterTag, AttackTypeTag);

	if (auto FoundMontagePtr = AttackMontages.Find(Key))
	{
		if (*FoundMontagePtr)
		{
			return *FoundMontagePtr;
		}
	}

	return nullptr;
}
