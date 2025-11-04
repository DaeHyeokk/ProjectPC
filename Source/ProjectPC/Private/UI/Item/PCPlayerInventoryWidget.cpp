// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Item/PCPlayerInventoryWidget.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/Image.h"

#include "Character/Unit/PCHeroUnitCharacter.h"
#include "GameFramework/PlayerState/PCPlayerState.h"
#include "Item/PCPlayerInventory.h"
#include "UI/Item/PCItemSlotWidget.h"


void UPCPlayerInventoryWidget::NativeDestruct()
{
	UnBindFromPlayerState();
	
	Super::NativeDestruct();
}

void UPCPlayerInventoryWidget::BindToPlayerState(APCPlayerState* NewPlayerState, bool IsOwner)
{
	if (!NewPlayerState) return;
	UnBindFromPlayerState();

	// 플레이어 정찰 중, 타인 인벤토리 간섭 방지를 위한 플래그
	bIsOwningInventory = IsOwner;

	// 플레이어 인벤토리 변화 구독
	PlayerInventory = NewPlayerState->GetPlayerInventory();
	PlayerInventory->OnInventoryUpdated.AddUObject(this, &UPCPlayerInventoryWidget::SetupInventory);
	
	for (int32 i = 1; i <= PlayerInventory->MaxInventorySlots; ++i)
	{
		FString SlotName = FString::Printf(TEXT("ItemSlot%d"), i);
		if (UWidget* FoundWidget = GetWidgetFromName(FName(*SlotName)))
		{
			// 아이템 슬롯 위젯 바인딩 (BP에서 10개 생성되어 있음)
			ItemSlots.Add(Cast<UPCItemSlotWidget>(FoundWidget));
		}
	}

	for (int32 i = 0; i < ItemSlots.Num(); ++i)
	{
		if (ItemSlots[i])
		{
			ItemSlots[i]->SetSlotIndex(i);
		}
	}

	SetupInventory();
}

void UPCPlayerInventoryWidget::UnBindFromPlayerState()
{
	// 플레이어 정찰 시, 인벤토리 위젯 바인딩을 바꿔주기 위한 언바인딩
	if (PlayerInventory)
	{
		PlayerInventory->OnInventoryUpdated.RemoveAll(this);
		PlayerInventory = nullptr;
	}

	for (int32 i = 0; i < ItemSlots.Num(); ++i)
	{
		if (ItemSlots[i])
		{
			ItemSlots[i]->RemoveItem();
		}
	}

	ItemSlots.Empty();

	if (DragImage)
	{
		DragImage->SetVisibility(ESlateVisibility::Collapsed);
		DragImage->SetBrushFromTexture(nullptr);
	}

	DragSlotIndex = -1;
	bIsDragging = false;
	bIsOwningInventory = true;
}

FReply UPCPlayerInventoryWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (!bIsOwningInventory) return FReply::Unhandled();
	Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);

	DragSlotIndex = GetSlotIndexAtMousePos(InGeometry, InMouseEvent.GetScreenSpacePosition());

	if (DragSlotIndex != -1 && PlayerInventory->GetInventory().IsValidIndex(DragSlotIndex))
	{
		if (ItemSlots.IsValidIndex(DragSlotIndex) && ItemSlots[DragSlotIndex] && ItemSlots[DragSlotIndex]->IsItemSet())
		{
			// 마우스가 클릭된 위치가 유효한 인벤토리 슬롯이면 드래그 활성화
			FEventReply Reply = UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton);
			return Reply.NativeReply;
		}
	}

	return FReply::Unhandled();
}

void UPCPlayerInventoryWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent,
	UDragDropOperation*& OutOperation)
{
	if (!bIsOwningInventory) return;
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

	if (DragSlotIndex == -1 || !ItemSlots.IsValidIndex(DragSlotIndex))
		return;

	if (auto DragDropOp = NewObject<UDragDropOperation>())
	{
		DragDropOp->Payload = this;
		DragDropOp->DefaultDragVisual = nullptr;
		DragDropOp->Pivot = EDragPivot::MouseDown;
		
		if (PlayerInventory)
		{
			// 드래그 중인 아이템 이미지 세팅 (마우스에 붙어 따라오는 이미지)
			if (DragImage && ItemSlots[DragSlotIndex])
			{
				if (UTexture2D* Thumbnail = ItemSlots[DragSlotIndex]->GetThumbnail())
				{
					// 드래그된 자리의 아이템 슬롯 제거
					ItemSlots[DragSlotIndex]->RemoveItem();
					DragImage->SetBrushFromTexture(Thumbnail);
					DragImage->SetVisibility(ESlateVisibility::HitTestInvisible);
				}
			}
		}

		bIsDragging = true;
		OutOperation = DragDropOp;
	}
}

bool UPCPlayerInventoryWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	if (!bIsOwningInventory) return false;
	Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);

	if (!InOperation || !PlayerInventory)
		return false;

	auto DropSlotIndex = GetSlotIndexAtMousePos(InGeometry, InDragDropEvent.GetScreenSpacePosition());

	// 드래그가 끝난 지점이 유효한 인벤토리의 아이템 슬롯일 때
	if (DropSlotIndex != -1 && PlayerInventory->GetInventory().IsValidIndex(DropSlotIndex)
		&& DropSlotIndex != DragSlotIndex)
	{
		PlayerInventory->EndDragItem(DragSlotIndex, DropSlotIndex);
	}
	// 드래그가 끝난 지점이 인벤토리가 아닐 때 (유닛 or 허공)
	else
	{
		LastMousePos = UWidgetLayoutLibrary::GetMousePositionOnPlatform();
		PlayerInventory->EndDragItem(DragSlotIndex, LastMousePos);
	}

	if (DragImage)
	{
		DragImage->SetVisibility(ESlateVisibility::Collapsed);
		DragImage->SetBrushFromTexture(nullptr);
	}

	// 아이템이 원래 자리로 돌아가야 하는 경우 해당 ItemSlot 다시 세팅
	if (DropSlotIndex == -1 || DropSlotIndex == DragSlotIndex
		|| !PlayerInventory->GetInventory().IsValidIndex(DropSlotIndex)
		|| (PlayerInventory->GetInventory().IsValidIndex(DragSlotIndex)
			&& PlayerInventory->GetInventory()[DropSlotIndex] == PlayerInventory->GetInventory()[DragSlotIndex]))
	{
		if (ItemSlots.IsValidIndex(DragSlotIndex) && ItemSlots[DragSlotIndex])
		{
			ItemSlots[DragSlotIndex]->SetItem(PlayerInventory->GetInventory()[DragSlotIndex]);
		}
	}

	DragSlotIndex = -1;
	bIsDragging = false;

	return true;
}

void UPCPlayerInventoryWidget::NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	if (!bIsOwningInventory) return;
	Super::NativeOnDragCancelled(InDragDropEvent, InOperation);

	if (!PlayerInventory)
		return;

	// 드래그가 취소된 경우, 드래그가 허공에서 끝난 것과 동일한 로직
	LastMousePos = UWidgetLayoutLibrary::GetMousePositionOnPlatform();
	PlayerInventory->EndDragItem(DragSlotIndex, LastMousePos);
	
	if (DragImage)
	{
		DragImage->SetVisibility(ESlateVisibility::Hidden);
		DragImage->SetBrushFromTexture(nullptr);
	}

	if (ItemSlots.IsValidIndex(DragSlotIndex) && ItemSlots[DragSlotIndex])
	{
		ItemSlots[DragSlotIndex]->SetItem(PlayerInventory->GetInventory()[DragSlotIndex]);
	}

	DragSlotIndex = -1;
	bIsDragging = false;
}

void UPCPlayerInventoryWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// 드래그 중인 아이템 이미지를 마우스 위치에 따라오게 만들기 위한 Tick 이벤트
	if (DragImage && bIsDragging)
	{
		const FVector2D MouseAbs = UWidgetLayoutLibrary::GetMousePositionOnPlatform();
		const FVector2D Local = MyGeometry.AbsoluteToLocal(MouseAbs);

		DragImage->SetRenderTranslation(Local - DragImage->GetDesiredSize() * 0.2f);
	}
}

void UPCPlayerInventoryWidget::SetupInventory()
{
	// 플레이어 인벤토리가 업데이트되면, 인벤토리에서 유효한 아이템 슬롯만 세팅하고 나머지는 제거
	for (int i = 0; i < ItemSlots.Num(); ++i)
	{
		if (ItemSlots[i])
		{
			if (PlayerInventory->GetInventory().IsValidIndex(i))
			{
				ItemSlots[i]->SetItem(PlayerInventory->GetInventory()[i]);
			}
			else
			{
				ItemSlots[i]->RemoveItem();
			}
		}
	}
}

int32 UPCPlayerInventoryWidget::GetSlotIndexAtMousePos(const FGeometry& InGeometry, const FVector2d& MousePos)
{
	for (int32 i = 0; i < ItemSlots.Num(); ++i)
	{
		if (ItemSlots[i])
		{
			FGeometry SlotGeometry = ItemSlots[i]->GetCachedGeometry();
			FVector2D LocalPosition = SlotGeometry.AbsoluteToLocal(MousePos);

			if (LocalPosition.X >= 0 && LocalPosition.X <= SlotGeometry.GetLocalSize().X &&
				LocalPosition.Y >= 0 && LocalPosition.Y <= SlotGeometry.GetLocalSize().Y)
			{
				return i;
			}
		}
	}

	return -1;
}
