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
struct FPCSynergyTierLine
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 Threshold = 0;          // 2, 4, 6 처럼

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText Line;                   // "최대 체력의 8%..." 같은 설명
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

	// ▼ 추가
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(MultiLine=true))
	FText Summary; // 상단 짧은 요약 ("마법사가 추가 주문력을 얻습니다.")

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(MultiLine=true))
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FPCSynergyTierLine> TierLines; // (2) … (4) … (6) …
	
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

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& E) override
	{

		Super::PostEditChangeProperty(E);
		Cached.Empty();
		for (const auto& R : Rows)
			if (R.SynergyTag.IsValid()) Cached.Add(R.SynergyTag, R);

	}
#endif

	
	bool Resolve(const FGameplayTag& Tag, FPCSynergyUIRow& Out) const
	{
		if (const FPCSynergyUIRow* Found = Cached.Find(Tag))
		{ Out = *Found; return true; }
		return false;
	}
	
	
	
	
};
