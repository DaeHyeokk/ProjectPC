// Fill out your copyright notice in the Description page of Project Settings.


#include "DataAsset/Player/PCDataAsset_PlayerPortrait.h"


TSoftObjectPtr<UTexture2D> UPCDataAsset_PlayerPortrait::GetPlayerPortrait(FGameplayTag CharacterTag)
{
	if (auto FoundPortraitPtr = PlayerPortraits.Find(CharacterTag))
	{
		if (*FoundPortraitPtr)
		{
			return *FoundPortraitPtr;
		}
	}

	return nullptr;
}
