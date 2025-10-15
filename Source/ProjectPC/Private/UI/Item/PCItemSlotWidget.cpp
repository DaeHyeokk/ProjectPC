// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Item/PCItemSlotWidget.h"

#include "Components/Image.h"
#include "Engine/AssetManager.h"


void UPCItemSlotWidget::SetSlotIndex(int32 NewSlotIndex)
{
	SlotIndex = NewSlotIndex;
}

void UPCItemSlotWidget::SetItem(const FPCItemData& NewItem)
{
	if (!Img_Item || !NewItem.IsValid()) return;
	
	FSoftObjectPath TexturePath = NewItem.ItemTexture.ToSoftObjectPath();
	FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
	TWeakObjectPtr<UPCItemSlotWidget> WeakThis = this;
	
	Streamable.RequestAsyncLoad(TexturePath, [WeakThis, TexturePath]()
	{
		if (WeakThis.IsValid())
		{
			if (UTexture2D* Texture = Cast<UTexture2D>(TexturePath.ResolveObject()))
			{
				WeakThis->Img_Item->SetBrushFromTexture(Texture);
				WeakThis->Img_Item->SetColorAndOpacity(FLinearColor::White);
				WeakThis->CachedThumbnail = Texture;
				WeakThis->bIsItemSet = true;
			}
		}
	});
}

void UPCItemSlotWidget::RemoveItem()
{
	if (!Img_Item) return;

	Img_Item->SetBrushFromTexture(nullptr);
	Img_Item->SetColorAndOpacity(FLinearColor::Black);
	bIsItemSet = false;
}

UTexture2D* UPCItemSlotWidget::GetThumbnail() const
{
	return CachedThumbnail;
}

bool UPCItemSlotWidget::IsItemSet() const
{
	return bIsItemSet;
}

bool UPCItemSlotWidget::IsUnderLocation(const FVector2D& ScreenPos) const
{
	if (!IsVisible())
		return false;

	const FGeometry& CachedGeo = GetCachedGeometry();

	// 스크린 좌표 → 위젯의 로컬 좌표로 변환
	const FVector2D LocalPos = CachedGeo.AbsoluteToLocal(ScreenPos);
	const FVector2D WidgetSize = CachedGeo.GetLocalSize();

	// 위젯의 로컬 좌표계 안에 들어있는지 확인
	FSlateRect LocalRect(0.f, 0.f, WidgetSize.X, WidgetSize.Y);

	return LocalRect.ContainsPoint(LocalPos);
}
