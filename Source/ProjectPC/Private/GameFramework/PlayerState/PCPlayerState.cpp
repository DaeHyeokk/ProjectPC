// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFramework/PlayerState/PCPlayerState.h"

#include "EngineUtils.h"
#include "Net/UnrealNetwork.h"

#include "AbilitySystem/Player/PCPlayerAbilitySystemComponent.h"
#include "AbilitySystem/Player/AttributeSet/PCPlayerAttributeSet.h"
#include "Character/Player/PCPlayerCharacter.h"
#include "GameFramework/HelpActor/PCPlayerBoard.h"
#include "Controller/Player/PCCombatPlayerController.h"


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
}

void APCPlayerState::SetPlayerBoard(APCPlayerBoard* InBoard)
{
	if (HasAuthority())
	{
		PlayerBoard = InBoard;
		if (PlayerBoard)
		{
			PlayerBoard->OwnerPlayerState = this;
		}
	}
}

void APCPlayerState::ResolvePlayerBoardOnClient()
{
	if (PlayerBoard) return;
	UWorld* World = GetWorld();
	for (TActorIterator<APCPlayerBoard> It(World); It; ++It)
	{
		if (It->PlayerIndex == SeatIndex)
		{
			PlayerBoard = *It;
			break;
		}
	}
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
				if (auto PlayerCharacter = Cast<APCPlayerCharacter>(GetPawn()))
				{
					PlayerCharacter->PlayerDie(9);
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

void APCPlayerState::ApplyRoundReward()
{
	if (!HasAuthority()) return;
	
	if (PlayerAbilitySystemComponent)
	{
		PlayerAbilitySystemComponent->ApplyPlayerRoundRewardEffect();
	}
	
	if (auto PC = Cast<APCCombatPlayerController>(GetPlayerController()))
	{
		PC->Server_ShopRefresh(0);
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

void APCPlayerState::OnRep_PlayerWinningStreak()
{
	OnWinningStreakUpdated.Broadcast();
}

void APCPlayerState::PlayerWin()
{
	if (!HasAuthority()) return;
	
	if (PlayerWinningStreak <= 0)
	{
		PlayerWinningStreak = 1;
	}
	else
	{
		PlayerWinningStreak++;
	}
}

void APCPlayerState::PlayerLose()
{
	if (!HasAuthority()) return;
	
	if (PlayerWinningStreak >= 0)
	{
		PlayerWinningStreak = -1;
	}
	else
	{
		PlayerWinningStreak--;
	}
}

int32 APCPlayerState::GetPlayerWinningStreak() const
{
	return PlayerWinningStreak;
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
	DOREPLIFETIME(APCPlayerState, PlayerBoard);
	DOREPLIFETIME(APCPlayerState, PlayerWinningStreak);
}

void APCPlayerState::OnRep_SeatIndex()
{
	ResolvePlayerBoardOnClient();
	
}
