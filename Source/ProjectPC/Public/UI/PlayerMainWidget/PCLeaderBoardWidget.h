// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerState/PCPlayerState.h"
#include "PCLeaderBoardWidget.generated.h"

struct FPlayerStandingRow;

class APCCombatGameState;
class UPCPlayerRowWidget;
class UVerticalBox;

/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCLeaderBoardWidget : public UUserWidget
{
	GENERATED_BODY()
	
private:
	UPROPERTY()
	APCCombatGameState* CachedGameState;

	UPROPERTY()
	// TMap<FString, TObjectPtr<UPCPlayerRowWidget>> PlayerMap;
	TMap<TWeakObjectPtr<APCPlayerState>, UPCPlayerRowWidget*> PlayerMap;

public:
	void BindToGameState(APCCombatGameState* NewGameState);

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UVerticalBox> PlayerBox;

	UPROPERTY(EditDefaultsOnly, Category = "PlayerRowWidgetClass")
	TSubclassOf<UUserWidget> PlayerRowWidgetClass;

public:
	// void SetupLeaderBoard(const TMap<FString, FPlayerStandingRow>& NewMap) const;
	void SetupLeaderBoard(const TArray<APCPlayerState*>& NewPlayerArray) const;
};
