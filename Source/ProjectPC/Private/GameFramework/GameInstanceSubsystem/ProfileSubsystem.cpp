// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFramework/GameInstanceSubsystem/ProfileSubsystem.h"

#include "Kismet/GameplayStatics.h"


void UProfileSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	SessionID = FGuid::NewGuid();
	
}

void UProfileSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UProfileSubsystem::SetUserID(const FString& UserSummitID)
{
	FString NewName = UserSummitID;
	NewName.TrimStartAndEndInline();
	
	CachedUserID = NewName;
}

