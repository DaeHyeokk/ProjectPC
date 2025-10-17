// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Item/PCItemSlotWidget.h"

#include "Components/Image.h"
#include "Engine/AssetManager.h"

#include "GameFramework/WorldSubsystem/PCItemManagerSubsystem.h"
#include "UI/Item/PCPlayerInventoryWidget.h"


void UPCItemSlotWidget::SetSlotIndex(int32 NewSlotIndex)
{
	SlotIndex = NewSlotIndex;
}

void UPCItemSlotWidget::SetItem(FGameplayTag NewItemTag)
{
	if (!Img_Item) return;

	if (const auto ItemManagerSubsystem = GetWorld()->GetSubsystem<UPCItemManagerSubsystem>())
	{
		if (const auto NewItem = ItemManagerSubsystem->GetItemData(NewItemTag))
		{
			if (NewItem->IsValid())
			{
				FSoftObjectPath TexturePath = NewItem->ItemTexture.ToSoftObjectPath();
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
		}
	}
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
