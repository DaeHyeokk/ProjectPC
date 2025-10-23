// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameFramework/Save/SaveGame_LocalPlayerProfile.h"
#include "ProfileSubsystem.generated.h"

/**
 * 
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnProfileStringChanged, const FString&, NewValue);
UCLASS()
class PROJECTPC_API UProfileSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// 영구 저장되는 UserID
	void SetUserID(const FString& UserSummitID);
	//const FString& GetUserID() const { return CachedUserID;}
	const FString& GetUserID() const { return CachedUserID;}
	bool HasDisplayName() const { return !CachedUserID.IsEmpty();}

	// 실행 고유 ID
	const FGuid& GetSessionID() const { return SessionID;}

private:

	// 런타임 캐시
	UPROPERTY()
	FString CachedUserID;
	
	UPROPERTY()
	FGuid SessionID;
};
