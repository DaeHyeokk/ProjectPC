// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Unit/GC/PCGameplayCue_UnitCombatText.h"

#include "AbilitySystem/Unit/ExecutionCalculation/PCUnitDamageExec.h"
#include "Character/Unit/PCBaseUnitCharacter.h"
#include "GameFramework/PlayerState/PCPlayerState.h"
#include "GameFramework/WorldSubsystem/PCUnitCombatTextSpawnSubsystem.h"
#include "Kismet/GameplayStatics.h"

bool UPCGameplayCue_UnitCombatText::OnExecute_Implementation(AActor* MyTarget,
                                                         const FGameplayCueParameters& Parameters) const
{
	// 클라 본인이 소환한 유닛의 전투 UI만 표시
	const APCBaseUnitCharacter* Instigator = Cast<APCBaseUnitCharacter>(Parameters.Instigator);
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	const APCPlayerState* LocalPS = PC ? Cast<APCPlayerState>(PC->PlayerState.Get()) : nullptr;
	if (!Instigator || !LocalPS || Instigator->GetTeamIndex() != LocalPS->SeatIndex)
		return false;
		
	if (!MyTarget)
		return false;
	
	const USceneComponent* AttachComp = MyTarget->FindComponentByClass<USkeletalMeshComponent>();
	if (!AttachComp)
		AttachComp = MyTarget->GetRootComponent();
	if (!AttachComp)
		return false;

	const float Value = Parameters.RawMagnitude;
	const bool bCrit = Parameters.NormalizedMagnitude >= 0.5f;
	const FGameplayTag CombatTextTypeTag = Parameters.AggregatedSourceTags.First();
	
	if (UWorld* World = MyTarget->GetWorld())
	{
		if (auto* Subsystem = World->GetSubsystem<UPCUnitCombatTextSpawnSubsystem>())
		{
			FCombatTextInitParams InitParams;
			InitParams.Value = Value;
			InitParams.bCritical = bCrit;
			InitParams.CombatTextTypeTag = CombatTextTypeTag;
			
			Subsystem->SpawnCombatText(AttachComp, InitParams);
			return true;
		}
	}
	
	return false;
}
