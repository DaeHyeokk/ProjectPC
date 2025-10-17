// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFramework/PlayerState/PCPlayerState.h"

#include "Net/UnrealNetwork.h"

#include "AbilitySystem/Player/PCPlayerAbilitySystemComponent.h"
#include "AbilitySystem/Player/AttributeSet/PCPlayerAttributeSet.h"
#include "Character/Player/PCPlayerCharacter.h"
#include "Component/PCSynergyComponent.h"


APCPlayerState::APCPlayerState()
{
	bReplicates = true;
	PlayerAbilitySystemComponent = CreateDefaultSubobject<UPCPlayerAbilitySystemComponent>("PlayerAbilitySystemComponent");
	PlayerAbilitySystemComponent->AddAttributeSetSubobject(CreateDefaultSubobject<UPCPlayerAttributeSet>(TEXT("PlayerAttributeSet")));
	PlayerAttributeSet = PlayerAbilitySystemComponent->GetSet<UPCPlayerAttributeSet>();
	
	PlayerAbilitySystemComponent->SetIsReplicated(true);
	PlayerAbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	NetUpdateFrequency = 100.f;
	MinNetUpdateFrequency = 60.f;

	AllStateTags.AddTag(PlayerGameplayTags::Player_State_Normal);
	AllStateTags.AddTag(PlayerGameplayTags::Player_State_Carousel);
	AllStateTags.AddTag(PlayerGameplayTags::Player_State_Dead);

	SynergyComponent = CreateDefaultSubobject<UPCSynergyComponent>(TEXT("SynergyComponent"));
}

void APCPlayerState::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		PlayerAbilitySystemComponent->InitAbilityActorInfo(this, this);
		PlayerAbilitySystemComponent->ApplyInitializedAbilities();
		PlayerAbilitySystemComponent->ApplyInitializedEffects();
		PlayerAbilitySystemComponent->AddLooseGameplayTag(PlayerGameplayTags::Player_State_Normal);
		CurrentStateTag = PlayerGameplayTags::Player_State_Normal;
	}
	else
	{
		PlayerAbilitySystemComponent->InitAbilityActorInfo(this, this);
	}
}

UAbilitySystemComponent* APCPlayerState::GetAbilitySystemComponent() const
{
	return PlayerAbilitySystemComponent; 
}

const UPCPlayerAttributeSet* APCPlayerState::GetAttributeSet() const
{
	return PlayerAttributeSet;
}

FGameplayTag APCPlayerState::GetCurrentStateTag() const
{
	return CurrentStateTag;
}

void APCPlayerState::ChangeState(FGameplayTag NewStateTag)
{
	if (PlayerAbilitySystemComponent && HasAuthority())
	{
		if (CurrentStateTag != PlayerGameplayTags::Player_State_Dead && NewStateTag.IsValid() && AllStateTags.HasTagExact(NewStateTag))
		{
			PlayerAbilitySystemComponent->RemoveLooseGameplayTags(AllStateTags);
			PlayerAbilitySystemComponent->AddLooseGameplayTag(NewStateTag);
			CurrentStateTag = NewStateTag;

			if (CurrentStateTag == PlayerGameplayTags::Player_State_Dead)
			{
				if (const auto PlayerCharacter = Cast<APCPlayerCharacter>(GetPawn()))
				{
					PlayerCharacter->PlayerDie();
				}
			}
		}
	}
}

void APCPlayerState::AddValueToPlayerStat(FGameplayTag PlayerStatTag, float Value) const
{
	if (PlayerAbilitySystemComponent && HasAuthority())
	{
		PlayerAbilitySystemComponent->ApplyPlayerEffects(PlayerStatTag, Value);
	}
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
	DOREPLIFETIME(APCPlayerState, ShopSlots);
	DOREPLIFETIME(APCPlayerState, PlayerLevel);
}
