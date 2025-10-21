// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Unit/PCCommonUnitCharacter.h"

#include "Net/UnrealNetwork.h"

void APCCommonUnitCharacter::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(APCCommonUnitCharacter, UnitTag);
}
