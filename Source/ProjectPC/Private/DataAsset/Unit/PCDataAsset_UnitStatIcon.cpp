// Fill out your copyright notice in the Description page of Project Settings.


#include "DataAsset/Unit/PCDataAsset_UnitStatIcon.h"


TSoftObjectPtr<UTexture2D> UPCDataAsset_UnitStatIcon::GetStatIcon(FGameplayTag StatTag)
{
	if (const TSoftObjectPtr<UTexture2D>* FoundPtr = StatIcons.Find(StatTag))
	{
		return *FoundPtr;
	}

	return nullptr;
}
