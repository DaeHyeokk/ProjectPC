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

	UFUNCTION(BlueprintPure, Category = "Profile")
	bool HasDisPlayName() const { return Profile && !Profile->DisplayName.IsEmpty(); }

	UFUNCTION(BlueprintPure, Category = "Profile")
	FString GetDisplayName() const { return Profile ? Profile->DisplayName : FString(); }

	UFUNCTION(BlueprintCallable, Category = "Profile")
	void SetDisplayName(const FString& InDisplayName);

	UPROPERTY(BlueprintAssignable, Category = "Profile|Event")
	FOnProfileStringChanged OnDisplayNameChanged;

	UFUNCTION(BlueprintPure, Category="Profile|Event")
	FString GetLastServerAddr() const {return Profile ? Profile->LastServerAddr : FString(); }

	UFUNCTION(BlueprintCallable, Category = "Profile|Evnet")
	void StartLastServerAddr(const FString& InAddr);

	UFUNCTION(BlueprintPure, Category = "Profile|Evnet")
	FString GetLobbyAddr() const;

	UPROPERTY(BlueprintAssignable, Category = "Profile|Event")
	FOnProfileStringChanged OnLastServerAddrChanged;

	UFUNCTION(BlueprintCallable, Category = "Profile|Save")
	void SaveNow();

	UFUNCTION(BlueprintCallable, Category = "Profile|Save")
	void LoadOrCreate();

private:
	UPROPERTY()
	USaveGame_LocalPlayerProfile* Profile = nullptr;

	void CreateNewProfile();

	static constexpr const TCHAR* SlotName() { return TEXT("LocalPlayerProfile");}
	static constexpr const TCHAR* KDefaultLobbyAddr = TEXT("127.0.0.1:7777");
	
};
