// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Net/Serialization/FastArraySerializer.h"

#include "PCSynergyCountRep.generated.h"
/**
 * 
 */

USTRUCT()
struct FSynergyCountEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()

	UPROPERTY()
	FGameplayTag Tag;
	UPROPERTY()
	int32 Count = 0;
};

USTRUCT()
struct FSynergyCountArray : public FFastArraySerializer
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FSynergyCountEntry> Entries;

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams)
	{
		return FastArrayDeltaSerialize(Entries, DeltaParams, *this);
	}

	void SetCount(const FGameplayTag& Tag, int32 NewCount)
	{
		if (!Tag.IsValid())
			return;

		if (FSynergyCountEntry* Found = Entries.FindByPredicate(
			[&](const FSynergyCountEntry& Entry){ return Entry.Tag == Tag; }))
		{
			Found->Count = NewCount;
			MarkItemDirty(*Found);
			return;
		}

		FSynergyCountEntry& Added = Entries.AddDefaulted_GetRef();
		Added.Tag = Tag;
		Added.Count = NewCount;
		MarkItemDirty(Added);
	}

	void RemoveByTag(const FGameplayTag& Tag)
	{
		const int32 Idx = Entries.IndexOfByPredicate(
			[&](const FSynergyCountEntry& Entry){ return Entry.Tag == Tag; });
		if (Idx != INDEX_NONE)
		{
			MarkItemDirty(Entries[Idx]);
			Entries.RemoveAtSwap(Idx);
			MarkArrayDirty();
		}
	}

	void ResetToMap(const TMap<FGameplayTag, int32>& Map)
	{
		Entries.Reset();
		for (const auto& KV : Map)
		{
			FSynergyCountEntry& Entry = Entries.AddDefaulted_GetRef();
			Entry.Tag = KV.Key;
			Entry.Count = KV.Value;
			MarkItemDirty(Entry);
		}
		MarkArrayDirty();
	}
};
template<> struct TStructOpsTypeTraits<FSynergyCountArray> : public TStructOpsTypeTraitsBase2<FSynergyCountArray>
{
	enum { WithNetDeltaSerializer = true };
};