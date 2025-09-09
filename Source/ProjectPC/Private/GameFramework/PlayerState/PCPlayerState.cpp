// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFramework/PlayerState/PCPlayerState.h"

#include "Net/UnrealNetwork.h"

#include "AbilitySystem/Player/PCPlayerAbilitySystemComponent.h"
#include "AbilitySystem/Player/AttributeSet/PCPlayerAttributeSet.h"


APCPlayerState::APCPlayerState()
{
	bReplicates = true;
	PlayerAbilitySystemComponent = CreateDefaultSubobject<UPCPlayerAbilitySystemComponent>("PlayerAbilitySystemComponent");
	PlayerAbilitySystemComponent->AddAttributeSetSubobject(CreateDefaultSubobject<UPCPlayerAttributeSet>(TEXT("PlayerAttributeSet")));
	PlayerAttributeSet = PlayerAbilitySystemComponent->GetSet<UPCPlayerAttributeSet>();

	PlayerAbilitySystemComponent->SetIsReplicated(true);
	PlayerAbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
}

void APCPlayerState::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		PlayerAbilitySystemComponent->InitAbilityActorInfo(this, this);
		PlayerAbilitySystemComponent->ApplyInitializedAbilities();
		PlayerAbilitySystemComponent->ApplyInitializedEffects();
	}
	else
	{
		PlayerAbilitySystemComponent->InitAbilityActorInfo(this, this);
	}
}


void APCPlayerState::SetFieldUnit(FGameplayTag UnitID, int UnitLevel, int32 TileIndex)
{
	if (!HasAuthority())
		return;
	bool bFound = false;
	for (auto& FieldUnitData : FieldUnit)
	{
		if (FieldUnitData.TileIndex == TileIndex)
		{
			FieldUnitData.UnitID = UnitID;
			FieldUnitData.TeamID = TeamID;
			FieldUnitData.UnitLevel = UnitLevel;
			bFound = true;
			break;
		}
	}
	if (!bFound)
		FieldUnit.Add(FUnitDataInBoard(UnitID, UnitLevel, TileIndex, TeamID));

	OnBoardUpdated.Broadcast();
}

void APCPlayerState::RemoveFieldAt(int32 TileIndex)
{
	if (!HasAuthority())
		return;
	FieldUnit.RemoveAll([&](const FUnitDataInBoard& FieldUnitData){return FieldUnitData.TileIndex == TileIndex;});
	OnBoardUpdated.Broadcast();
}

void APCPlayerState::SetBenchUnit(FGameplayTag UnitID, int UnitLevel, int32 BenchIndex)
{
	if (!HasAuthority())
		return;
	bool bFound = false;
	for (FUnitDataInBoard& BenchUnitData : BenchUnit)
	{
		BenchUnitData.UnitID = UnitID;
		BenchUnitData.TeamID = TeamID;
		BenchUnitData.UnitLevel = UnitLevel;
		bFound = true;
		break;
	}
	if (!bFound)
		BenchUnit.Add(FUnitDataInBoard(UnitID, UnitLevel, BenchIndex, BenchIndex));
	OnBoardUpdated.Broadcast();
}

void APCPlayerState::RemoveBench(int32 BenchIndex)
{
	if (!HasAuthority())
		return;
	BenchUnit.RemoveAll([&](const FUnitDataInBoard& BenchUnitData){return BenchUnitData.TileIndex == BenchIndex;});
	OnBoardUpdated.Broadcast();
}

void APCPlayerState::OnRep_UnitData()
{
	OnBoardUpdated.Broadcast();
}

UAbilitySystemComponent* APCPlayerState::GetAbilitySystemComponent() const
{
	return PlayerAbilitySystemComponent; 
}

const UPCPlayerAttributeSet* APCPlayerState::GetAttributeSet() const
{
	return PlayerAttributeSet;
}

void APCPlayerState::OnRep_ShopSlots()
{
	OnShopSlotsUpdated.Broadcast();
}

void APCPlayerState::Client_ForceShopSlotsUpdate_Implementation()
{
	OnRep_ShopSlots();
}

void APCPlayerState::SetShopSlots(const TArray<FPCShopUnitData>& NewSlots)
{
	if (ShopSlots == NewSlots)
		Client_ForceShopSlotsUpdate();
	else
		ShopSlots = NewSlots;

	if (HasAuthority())
		OnRep_ShopSlots();
}

const TArray<FPCShopUnitData>& APCPlayerState::GetShopSlots()
{
	return ShopSlots;
}

void APCPlayerState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(APCPlayerState, bIsReady);
	DOREPLIFETIME(APCPlayerState, LocalUserId);
	DOREPLIFETIME(APCPlayerState, bIsLeader);
	DOREPLIFETIME(APCPlayerState, SeatIndex);
	DOREPLIFETIME(APCPlayerState, bIdentified);
	DOREPLIFETIME(APCPlayerState, FieldUnit);
	DOREPLIFETIME(APCPlayerState, BenchUnit);
	DOREPLIFETIME(APCPlayerState, TeamID);
	DOREPLIFETIME(APCPlayerState, ShopSlots);
}
