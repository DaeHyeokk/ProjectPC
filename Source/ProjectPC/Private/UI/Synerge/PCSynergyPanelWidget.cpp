// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Synerge/PCSynergyPanelWidget.h"

#include "Component/PCSynergyComponent.h"
#include "Components/VerticalBox.h"
#include "UI/Synerge/PCSynergySlotWidget.h"


void UPCSynergyPanelWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (!SynergyComponent.IsValid()) return;

	OnChangedHandle = SynergyComponent->OnSynergyCountsChanged.AddUObject(this, &UPCSynergyPanelWidget::HandlesSynergyChanged);
	HandlesSynergyChanged((SynergyComponent->GetSynergySnapShot()));
}

void UPCSynergyPanelWidget::NativeDestruct()
{
	if (SynergyComponent.IsValid() && OnChangedHandle.IsValid())
	{
		SynergyComponent->OnSynergyCountsChanged.Remove(OnChangedHandle);
	}
	Super::NativeDestruct();
}

void UPCSynergyPanelWidget::SetSynergyComponent(UPCSynergyComponent* Component)
{
	SynergyComponent = Component;
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


