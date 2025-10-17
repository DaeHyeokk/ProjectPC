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

	UPROPERTY(EditDefaultsOnly, Category="Mesh")
	TObjectPtr<USkeletalMesh> Mesh;

	// 유닛 타입을 AppearanceChange 지정할 경우 보여지는 속성 (레벨에 따라 변하는 Mesh)
	UPROPERTY(EditDefaultsOnly, Category="Mesh", meta=(EditCondition="ClassType==EUnitClassType::Hero_AppearanceChange"))
	TObjectPtr<USkeletalMesh> Mesh_Level2;
	// 유닛 타입을 AppearanceChange 지정할 경우 보여지는 속성 (레벨에 따라 변하는 Mesh)
	UPROPERTY(EditDefaultsOnly, Category="Mesh", meta=(EditCondition="ClassType==EUnitClassType::Hero_AppearanceChange"))
	TObjectPtr<USkeletalMesh> Mesh_Level3;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UAnimInstance> AnimBP;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UPCDataAsset_BaseUnitData> UnitDataAsset;

	// Creep이 아닌 Hero 타입으로 지정할 경우 보여지는 속성 (이동할 때 나타나는 Preview Hero Unit의 AnimBP)
	UPROPERTY(EditDefaultsOnly, Category="Preview AnimBP", meta=(EditCondition="ClassType!=EUnitClassType::Creep"))
	TSubclassOf<UAnimInstance> PreviewHeroAnimBP;

	// Creep이 아닌 Hero 타입으로 지정할 경우 보여지는 속성 (Carousel Hero에 사용하는 AnimBP)
	UPROPERTY(EditDefaultsOnly, Category="Carousel AnimBP", meta=(EditCondition="ClassType!=EUnitClassType::Creep"))
	TSubclassOf<UAnimInstance> CarouselHeroAnimBP;
};
