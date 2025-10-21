// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PCSynergyPanelWidget.generated.h"

struct FSynergyData;
class UPCSynergyComponent;
class UPCSynergySlotWidget;
class UVerticalBox;
/**
 * 
 */
UCLASS()
class PROJECTPC_API UPCSynergyPanelWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	void SetSynergyComponent(UPCSynergyComponent* Component);

protected:
	UPROPERTY(meta = (BindWidget))
	UVerticalBox* Synergy_List;

	UPROPERTY(EditAnywhere, Category = "Class")
	TSubclassOf<UPCSynergySlotWidget> SlotClass;

private:

	UPROPERTY()
	TWeakObjectPtr<UPCSynergyComponent> SynergyComponent;

	FDelegateHandle OnChangedHandle;

	UFUNCTION()
	void HandlesSynergyChanged(const TArray<FSynergyData>& Data);

	void Rebuild(const TArray<FSynergyData>& Data);
	
	
	
	
	
	
};
