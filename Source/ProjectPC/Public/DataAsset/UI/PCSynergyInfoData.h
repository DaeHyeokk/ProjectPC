// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "PCSynergyInfoData.generated.h"



USTRUCT(BlueprintType)
struct FPCSynergyUITierSet
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FSlateBrush> TierBorders;
	
};

USTRUCT(BlueprintType)
struct FPCSynergyUIRow
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag SynergyTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FSlateBrush Icon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FPCSynergyUITierSet TierSet;
	
};

UCLASS()
class PROJECTPC_API UPCSynergyInfoData : public UDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FPCSynergyUIRow> Rows;

	UPROPERTY(Transient)
	TMap<FGameplayTag, FPCSynergyUIRow> Cached;

	virtual void PostLoad() override
	{
		Super::PostLoad();
		Cached.Empty();
		for (const auto& R : Rows)
		{
			if (R.SynergyTag.IsValid())
				Cached.Add(R.SynergyTag, R);
		}
	}
	virtual void PostEditChangeProperty(FPropertyChangedEvent& E) override
	{
#if WITH_EDITOR
		Super::PostEditChangeProperty(E);
		Cached.Empty();
		for (const auto& R : Rows)
			if (R.SynergyTag.IsValid()) Cached.Add(R.SynergyTag, R);
#endif
	}

	bool Resolve(const FGameplayTag& Tag, FPCSynergyUIRow& Out) const
	{
		if (const FPCSynergyUIRow* Found = Cached.Find(Tag))
		{ Out = *Found; return true; }
		return false;
	}
	
	
	
	
};
