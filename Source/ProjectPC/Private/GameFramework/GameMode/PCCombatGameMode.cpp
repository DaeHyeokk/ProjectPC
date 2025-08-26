// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFramework/GameMode/PCCombatGameMode.h"

#include "BaseGameplayTags.h"
#include "GameFramework/GameState.h"
#include "GameFramework/PlayerState/PCPlayerState.h"
#include "SubSystem/PCUnitGERegistrySubsystem.h"


void APCCombatGameMode::PostSeamlessTravel()
{
	Super::PostSeamlessTravel();
	if (auto* GS = GetGameState<AGameState>())
	{
		for (APlayerState* PS : GS->PlayerArray)
			if (auto* PCPlayerState = Cast<APCPlayerState>(PS))
				PCPlayerState->bIsReady = false;
	}
}

void APCCombatGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (!UnitGEDictionary)
		return;
	
	const UGameInstance* GI = GetWorld()->GetGameInstance();
	if (auto* UnitGERegistrySubsystem = GI->GetSubsystem<UPCUnitGERegistrySubsystem>())
	{
		FGameplayTagContainer PreloadGEClassTag;
		PreloadGEClassTag.AddTag(GameplayEffectTags::GE_Class_HealthChange);
		PreloadGEClassTag.AddTag(GameplayEffectTags::GE_Class_ManaChange);
		
		UnitGERegistrySubsystem->InitializeUnitGERegistry(UnitGEDictionary, PreloadGEClassTag);
	}
	
}
