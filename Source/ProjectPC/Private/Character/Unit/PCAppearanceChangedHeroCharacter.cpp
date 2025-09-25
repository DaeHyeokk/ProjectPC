// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Unit/PCAppearanceChangedHeroCharacter.h"

#include "GameFramework/WorldSubsystem/PCUnitSpawnSubsystem.h"


void APCAppearanceChangedHeroCharacter::OnRep_HeroLevel()
{
	Super::OnRep_HeroLevel();

	// 유닛이 Spawn 되면 UnitTag 값이 세팅됨 -> 클라에서 OnRep_UnitTag 호출
	// -> 클라에서 보여줄 유닛 데이터 세팅 (메쉬, AnimBP, StatusBar UI)
	if (UWorld* W = GetWorld())
	{
		if (auto* SpawnSubSystem = W->GetSubsystem<UPCUnitSpawnSubsystem>())
		{
			const auto* Definition = SpawnSubSystem->ResolveDefinition(UnitTag);
			if (!Definition)
			{
				SpawnSubSystem->EnsureConfigFromGameState();
				Definition = SpawnSubSystem->ResolveDefinition(UnitTag);
			}
			if (Definition)
			{
				USkeletalMesh* SkMesh = nullptr;
				switch (HeroLevel)
				{
				case 2:
					SkMesh = Definition->Mesh_Level2;
					break;

				case 3:
					SkMesh = Definition->Mesh_Level3;

				default:
					break;
				}

				if (GetMesh() && SkMesh)
					GetMesh()->SetSkeletalMesh(SkMesh);
			}
		}
	}
}
