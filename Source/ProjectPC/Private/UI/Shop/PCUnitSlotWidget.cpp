// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Shop/PCUnitSlotWidget.h"

#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Engine/Texture2D.h"


bool UPCUnitSlotWidget::Initialize()
{
	bool SuperSuccess = Super::Initialize();
	if (!SuperSuccess) return false;

	if (!Btn_UnitSlot) return false;
	Btn_UnitSlot->OnClicked.AddDynamic(this, &UPCUnitSlotWidget::OnClickedUnitSlot);

	return true;
}

void UPCUnitSlotWidget::Setup(FPCShopUnitData UnitData)
{
	if (!Text_UnitName || !Text_Cost || !Img_UnitThumbnail || !Img_CostBorder) return;
	
	Text_UnitName->SetText(FText::FromName(UnitData.UnitName));
	Text_Cost->SetText(FText::AsNumber(UnitData.UnitCost));
	Img_UnitThumbnail->SetBrushFromTexture(UnitData.UnitTexture);

	UE_LOG(LogTemp, Warning, TEXT("%s"), *UnitData.UnitName.ToString());
	
	UTexture2D* BorderTexture = nullptr;

	switch (UnitData.UnitCost)
	{
	case 1:
		BorderTexture = Cost1Border;
		break;
	case 2:
		BorderTexture = Cost2Border;
		break;
	case 3:
		BorderTexture = Cost3Border;
		break;
	case 4:
		BorderTexture = Cost4Border;
		break;
	case 5:
		BorderTexture = Cost5Border;
		break;
	default:
		break;
	}

	if (BorderTexture)
		Img_CostBorder->SetBrushFromTexture(BorderTexture);
}

void UPCUnitSlotWidget::OnClickedUnitSlot()
{
}
