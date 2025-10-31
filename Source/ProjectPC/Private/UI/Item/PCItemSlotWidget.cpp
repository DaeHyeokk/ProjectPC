// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Item/PCItemSlotWidget.h"

#include "Components/Image.h"
#include "Engine/AssetManager.h"

#include "GameFramework/WorldSubsystem/PCItemManagerSubsystem.h"
#include "UI/Item/PCItemRecipeWidget.h"
#include "UI/Item/PCItemInfoWidget.h"


bool UPCItemSlotWidget::Initialize()
{
	if (!Super::Initialize()) return false;

	ItemRecipeWidget = CreateWidget<UPCItemRecipeWidget>(GetWorld(), ItemRecipeWidgetClass);
	if (!ItemRecipeWidget) return false;
	ItemInfoWidget = CreateWidget<UPCItemInfoWidget>(GetWorld(), ItemInfoWidgetClass);
	if (!ItemInfoWidget) return false;
	
	return true;
}

void UPCItemSlotWidget::SetSlotIndex(int32 NewSlotIndex)
{
	SlotIndex = NewSlotIndex;
}

void UPCItemSlotWidget::SetItem(FGameplayTag NewItemTag)
{
	if (!Img_Item) return;

	const FGameplayTag BaseTag = FGameplayTag::RequestGameplayTag(TEXT("Item.Type.Base"));
	bIsBaseItem = NewItemTag.MatchesTag(BaseTag);

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
				
				if (ItemInfoWidget)
				{
					ItemInfoWidget->Setup(NewItemTag);
				}
				
				if (ItemRecipeWidget && bIsBaseItem)
				{
					ItemRecipeWidget->Setup(NewItemTag);
				}
			}
		}
	}
}

void UPCItemSlotWidget::RemoveItem()
{
	if (!Img_Item || !ItemRecipeWidget) return;

	Img_Item->SetBrushFromTexture(nullptr);
	Img_Item->SetColorAndOpacity(FLinearColor::Black);
	bIsItemSet = false;
	bIsBaseItem = false;
}

UTexture2D* UPCItemSlotWidget::GetThumbnail() const
{
	return CachedThumbnail;
}

bool UPCItemSlotWidget::IsItemSet() const
{
	return bIsItemSet;
}

void UPCItemSlotWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);

	if (!bIsItemSet || !ItemRecipeWidget || !ItemInfoWidget) return;

	ItemInfoWidget->AddToViewport(9999);
	ItemInfoWidget->SetVisibility(ESlateVisibility::HitTestInvisible);

	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		int32 ViewportX, ViewportY;
		PC->GetViewportSize(ViewportX, ViewportY);

		FVector2D MousePos;
		PC->GetMousePosition(MousePos.X, MousePos.Y);

		bool bIsRightHalf = MousePos.X > ViewportX * 0.5f;
		float XOffset = bIsRightHalf ? -370.0f : 70.0f;
		float YOffset = -50.0f - (SlotIndex * 20.0f);

		ItemInfoWidget->SetPositionInViewport(InGeometry.GetAbsolutePosition() + FVector2D(XOffset, YOffset));
	}
}

FReply UPCItemSlotWidget::NativeOnMouseWheel(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (!bIsItemSet | !ItemRecipeWidget || !ItemInfoWidget)
	{
		return FReply::Unhandled();
	}

	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		int32 ViewportX, ViewportY;
		PC->GetViewportSize(ViewportX, ViewportY);

		FVector2D MousePos;
		PC->GetMousePosition(MousePos.X, MousePos.Y);

		bool bIsRightHalf = MousePos.X > ViewportX * 0.5f;
		float XOffset = bIsRightHalf ? -370.0f : 70.0f;
		float YOffset = -50.0f - (SlotIndex * 25.0f);

		const float WheelDelta = InMouseEvent.GetWheelDelta();

		if (WheelDelta > 0.f)
		{
			if (ItemRecipeWidget->IsInViewport())
			{
				ItemRecipeWidget->RemoveFromParent();

				ItemInfoWidget->AddToViewport(9999);
				ItemInfoWidget->SetVisibility(ESlateVisibility::HitTestInvisible);

				ItemInfoWidget->SetPositionInViewport(InGeometry.GetAbsolutePosition() + FVector2D(XOffset, YOffset));
			}
		}
		else
		{
			if (ItemInfoWidget->IsInViewport() && bIsBaseItem)
			{
				ItemInfoWidget->RemoveFromParent();

				ItemRecipeWidget->AddToViewport(9999);
				ItemRecipeWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
				ItemRecipeWidget->SetDesiredSizeInViewport(FVector2D(320.0f, 520.0f));

				float RecipeYOffset = -50.0f - (SlotIndex * 30.0f);
				ItemRecipeWidget->SetPositionInViewport(InGeometry.GetAbsolutePosition() + FVector2D(XOffset, RecipeYOffset));
			}
		}
	}

	return FReply::Handled();
}

void UPCItemSlotWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);

	if (ItemRecipeWidget)
	{
		ItemRecipeWidget->RemoveFromParent();
	}

	if (ItemInfoWidget)
	{
		ItemInfoWidget->RemoveFromParent();
	}
}
