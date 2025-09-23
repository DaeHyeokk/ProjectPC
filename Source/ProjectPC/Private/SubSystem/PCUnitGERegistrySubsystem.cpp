// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFramework/WorldSubsystem/PCUnitGERegistrySubsystem.h"

#include "BaseGameplayTags.h"
#include "Engine/AssetManager.h"


void UPCUnitGERegistrySubsystem::InitializeUnitGERegistry(UPCDataAsset_UnitGEDictionary* InUnitGEDict,
                                                          const FGameplayTagContainer& PreloadGEClassKeys)
{
	UnitGEDict = InUnitGEDict;
	if (!UnitGEDict)
		return;

	TArray<FSoftObjectPath> Paths;
	for (const FGameplayTag& Key : PreloadGEClassKeys)
	{
		if (const TSoftClassPtr<UGameplayEffect>* GEClass = UnitGEDict->Entries.Find(Key))
		{
			if (GEClass->IsValid())
			{
				Paths.Add(GEClass->ToSoftObjectPath());
			}
		}
	}
	UAssetManager::GetStreamableManager().RequestSyncLoad(Paths);

	for (const FGameplayTag& Key : PreloadGEClassKeys)
	{
		if (const TSoftClassPtr<UGameplayEffect>* GEClass = UnitGEDict->Entries.Find(Key))
		{
			if (UClass* C = GEClass->Get())
				CachedGEClasses.Add(Key, C);
		}
	}
}

TSubclassOf<UGameplayEffect> UPCUnitGERegistrySubsystem::GetGEClass(const FGameplayTag& GEClassKey)
{
	if (const TWeakObjectPtr<UClass>* Found = CachedGEClasses.Find(GEClassKey))
	{
		if (Found->IsValid())
			return Found->Get();
		
	}

	if (!UnitGEDict)
		return nullptr;

	if (const TSoftClassPtr<UGameplayEffect>* GEClass = UnitGEDict->Entries.Find(GEClassKey))
	{
		if (UClass* Loaded = GEClass->LoadSynchronous())
		{
			CachedGEClasses.Add(GEClassKey, Loaded);
			return Loaded;
		}
	}

	return nullptr;
}

UGameplayEffect* UPCUnitGERegistrySubsystem::GetGE_CDO(const FGameplayTag& GEClassKey)
{
	if (UClass* C = GetGEClass(GEClassKey))
		return C->GetDefaultObject<UGameplayEffect>();

	return nullptr;
}
