// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Synerge/PCSynergyPanelWidget.h"

#include "Component/PCSynergyComponent.h"
#include "Components/VerticalBox.h"
#include "UI/Synerge/PCSynergySlotWidget.h"


void UPCSynergyPanelWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
}

void UPCSynergyPanelWidget::NativeDestruct()
{
	if (SynergyComponent.IsValid() && OnChangedHandle.IsValid())
	{
		SynergyComponent->OnSynergyCountsChanged.Remove(OnChangedHandle);
	}
	Super::NativeDestruct();
}

void UPCSynergyPanelWidget::SynergyComponentBinding(UPCSynergyComponent* Component)
{
	if (!Component) return;

	OnChangedHandle = Component->OnSynergyCountsChanged.AddUObject(this, &UPCSynergyPanelWidget::HandlesSynergyChanged);
	HandlesSynergyChanged((Component->GetSynergySnapShot()));
}


void UPCSynergyPanelWidget::HandlesSynergyChanged(const TArray<FSynergyData>& Data)
{
	Rebuild(Data);
}

void UPCSynergyPanelWidget::Rebuild(const TArray<FSynergyData>& Data)
{
	if (!Synergy_List) return;

	Synergy_List->ClearChildren();

	if (!SlotClass) return;

	for (const FSynergyData& D : Data)
	{
		UPCSynergySlotWidget* InSlot = CreateWidget<UPCSynergySlotWidget>(this, SlotClass);
		if (!Slot) continue;

		InSlot->SetData(D);
		Synergy_List->AddChild(InSlot);
	}
}


