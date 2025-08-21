// Fill out your copyright notice in the Description page of Project Settings.


#include "SubSystem/PCUnitGERegistrySubsystem.h"

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

UClass* UPCUnitGERegistrySubsystem::GetGEClass(const FGameplayTag& GEClassKey)
{
	if (const TWeakObjectPtr<UClass>* Found = CachedGEClasses.Find(GEClassKey))
		if (UClass* C = Found->Get())
			return C;

	if (!UnitGEDict)
		return nullptr;

	if (const TSoftClassPtr<UGameplayEffect>* GEClass = UnitGEDict->Entries.Find(GEClassKey))
	{
		UAssetManager::GetStreamableManager().RequestSyncLoad(GEClass->ToSoftObjectPath());
		if (UClass* C = GEClass->Get())
		{
			CachedGEClasses.Add(GEClassKey, C);
			return C;
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
