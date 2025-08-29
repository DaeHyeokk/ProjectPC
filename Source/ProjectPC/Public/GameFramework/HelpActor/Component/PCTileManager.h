// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PCTileManager.generated.h"

UENUM(BlueprintType)
enum class ETileType : uint8
{
	Filed,
	Bench,
	Battle
};

UENUM(BlueprintType)
enum class EGameMode : uint8
{
	Setup,
	Battle
};

USTRUCT()
struct FTileRange
{
	GENERATED_BODY()
	int32 Start = 0;
	int32 End = -1;
	FTileRange(){}
	FTileRange(int32 start, int32 End) : Start(start), End(End){}
	bool IsValid() const { return End >= Start; }
	bool Contains(int32 I) const {return I>=Start && I<=End;}
	int32 Size() const { return IsValid() ? (End - Start + 1) : 0; }
};

USTRUCT(BlueprintType)
struct FBoardConfig
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float TileSpacing = 100.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float NeighborThreshold = 250.f;
	
};

USTRUCT(BlueprintType)
struct FTileNode
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 Index = -1;
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTPC_API UPCTileManager : public UActorComponent
{
	GENERATED_BODY()

public:	
	
	UPCTileManager();

protected:
	
	virtual void BeginPlay() override;

public:	
	

		
	
};
