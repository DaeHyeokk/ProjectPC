// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PCDataAsset_UnitDefinition.generated.h"

class UPCDataAsset_BaseUnitData;
class APCBaseUnitCharacter;

UENUM(BlueprintType)
enum class EUnitClassType : uint8
{
	Creep,
	Hero_AppearanceChange,
	Hero_AppearanceFixed
};

/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCDataAsset_UnitDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EUnitClassType ClassType = EUnitClassType::Creep;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<USkeletalMesh> Mesh;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UAnimInstance> AnimBP;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UPCDataAsset_BaseUnitData> UnitDataAsset;
};
