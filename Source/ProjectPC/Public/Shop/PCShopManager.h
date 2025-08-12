// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Shop/PCShopUnitData.h"
#include "PCShopManager.generated.h"

/**
 * 
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTPC_API UPCShopManager : public UActorComponent
{
	GENERATED_BODY()
	
protected:
	// 유닛 데이터가 저장된 DataTable 가져옴
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DataTable")
	UDataTable* UnitDataTable;

public:
	// Row네임을 통해 해당 UnitData를 찾아서 아웃파라미터로 넘기고, bool값 리턴
	UFUNCTION(BlueprintCallable, Category = "DataTable")
	bool GetUnitDataByUnitName(FName RowName, FPCShopUnitData& OutUnitData);
};
