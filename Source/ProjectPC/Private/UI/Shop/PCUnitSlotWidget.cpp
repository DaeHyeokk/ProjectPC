// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Shop/PCUnitSlotWidget.h"

#include "Components/Button.h"


bool UPCUnitSlotWidget::Initialize()
{
	bool SuperSuccess = Super::Initialize();
	if (!SuperSuccess) return false;

	if (Btn_UnitSlot == nullptr) return false;
	Btn_UnitSlot->OnClicked.AddDynamic(this, &UPCUnitSlotWidget::OnClickedUnitSlot);

	return true;
}

void UPCUnitSlotWidget::Setup(FName UnitName)
{
}

void UPCUnitSlotWidget::OnClickedUnitSlot()
{
}
