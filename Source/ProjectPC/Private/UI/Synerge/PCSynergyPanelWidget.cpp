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

	SynergyComponent = Component;

	OnChangedHandle = Component->OnSynergyCountsChanged.AddUObject(this, &UPCSynergyPanelWidget::HandlesSynergyChanged);
	HandlesSynergyChanged((Component->GetSynergySnapShot()));
}


void UPCSynergyPanelWidget::HandlesSynergyChanged(const TArray<FSynergyData>& Data)
{
	Rebuild(Data);
}

void UPCSynergyPanelWidget::Rebuild(const TArray<FSynergyData>& Data)
{
	if (!Synergy_List || !SlotClass) return;

	Synergy_List->ClearChildren();

	// 1) 정렬: 티어 인덱스 내림차순 -> 카운트 내림차순 -> 태그명 오름차순
	TArray<FSynergyData> Sorted = Data;
	Sorted.StableSort([](const FSynergyData& A, const FSynergyData& B)
	{
		if (A.TierIndex != B.TierIndex)
			return A.TierIndex > B.TierIndex;         // 높은 티어 먼저
		if (A.Count != B.Count)
			return A.Count > B.Count;                 // 카운트 큰 것 먼저
		return A.SynergyTag.ToString() < B.SynergyTag.ToString(); // 이름순
	});

	// 2) 슬롯 생성
	for (const FSynergyData& D : Sorted)
	{
		UPCSynergySlotWidget* InSlot = CreateWidget<UPCSynergySlotWidget>(this, SlotClass);
		if (!InSlot) continue;

		InSlot->SetData(D);
		Synergy_List->AddChild(InSlot);
	}
}


