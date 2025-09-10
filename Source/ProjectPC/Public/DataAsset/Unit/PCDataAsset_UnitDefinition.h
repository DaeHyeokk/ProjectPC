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

	// Creep이 아닌 Hero 타입으로 지정할 경우 보여지는 속성 (이동할 때 나타나는 Preview Hero Unit의 AnimBP)
	UPROPERTY(EditDefaultsOnly, Category="Preview AnimBP", meta=(EditCondition="ClassType!=EUnitClassType::Creep"))
	TSubclassOf<UAnimInstance> PreviewHeroAnimBP;
};
