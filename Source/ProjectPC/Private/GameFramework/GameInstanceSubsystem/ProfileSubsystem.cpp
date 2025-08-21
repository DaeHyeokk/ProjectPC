// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFramework/GameInstanceSubsystem/ProfileSubsystem.h"

#include "Kismet/GameplayStatics.h"
#include "Tools/UEdMode.h"


void UProfileSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	LoadOrCreate();

	if (Profile && Profile->LastServerAddr.IsEmpty())
	{
		Profile->LastServerAddr = KDefaultLobbyAddr;
		SaveNow();
	}
}

void UProfileSubsystem::Deinitialize()
{
	SaveNow();
	Super::Deinitialize();
}

void UProfileSubsystem::SetDisplayName(const FString& InDisplayName)
{
	if (!Profile)
	{
		CreateNewProfile();
	}

	FString NewName = InDisplayName;
	NewName.TrimStartAndEndInline();

	if (Profile->DisplayName.Equals(NewName, ESearchCase::CaseSensitive))
	{
		return;
	}

	Profile->DisplayName = NewName;
	SaveNow();
	OnDisplayNameChanged.Broadcast(Profile->DisplayName);
}

void UProfileSubsystem::StartLastServerAddr(const FString& InAddr)
{
	if (!Profile)
	{
		CreateNewProfile();
	}
	FString NewAddr = InAddr;
	NewAddr.TrimStartAndEndInline();

	if (Profile->LastServerAddr.Equals(NewAddr, ESearchCase::CaseSensitive))
	{
		return;
	}

	Profile->LastServerAddr = NewAddr.IsEmpty() ? KDefaultLobbyAddr : NewAddr;
	SaveNow();
	OnLastServerAddrChanged.Broadcast(Profile->LastServerAddr);
}

FString UProfileSubsystem::GetLobbyAddr() const
{
	if(!Profile) return KDefaultLobbyAddr;
	return Profile->LastServerAddr.IsEmpty() ? KDefaultLobbyAddr : Profile->LastServerAddr;
}

void UProfileSubsystem::SaveNow()
{
	if (!Profile) return;
	UGameplayStatics::SaveGameToSlot(Profile, SlotName(), 0 );
}

void UProfileSubsystem::LoadOrCreate()
{
	if (USaveGame* Loaded = UGameplayStatics::LoadGameFromSlot(SlotName(),0))
	{
		Profile = Cast<USaveGame_LocalPlayerProfile>(Loaded);
	}
	if (!Profile)
	{
		CreateNewProfile();
	}
}

void UProfileSubsystem::CreateNewProfile()
{
	Profile = Cast<USaveGame_LocalPlayerProfile>(UGameplayStatics::CreateSaveGameObject(USaveGame_LocalPlayerProfile::StaticClass()));
	if (!Profile) return;

	Profile->DisplayName.Empty();
	Profile->LastServerAddr = KDefaultLobbyAddr;
	Profile->CreatedAtUTC = FDateTime::UtcNow();

	SaveNow();
}
