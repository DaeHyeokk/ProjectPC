// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PlayerMainWidget/PCPlayerRowWidget.h"

#include "AbilitySystemComponent.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"

#include "AbilitySystem/Player/AttributeSet/PCPlayerAttributeSet.h"
#include "Controller/Player/PCCombatPlayerController.h"
#include "GameFramework/PlayerState/PCPlayerState.h"


bool UPCPlayerRowWidget::Initialize()
{
	if (!Super::Initialize())
		return false;

	if (!Btn_CameraSwitch) return false;
	Btn_CameraSwitch->OnClicked.AddDynamic(this, &UPCPlayerRowWidget::PlayerPatrol);

	return true;
}

void UPCPlayerRowWidget::SetupPlayerInfo(APCPlayerState* NewPlayerState)
{
	if (!NewPlayerState) return;
	if (!PlayerName || !PlayerHP || !CircularHPBar || !Img_Portrait) return;

	CachedPlayerState = NewPlayerState;

	if (auto PC = GetOwningPlayer())
	{
		if (auto PS = PC->GetPlayerState<APCPlayerState>())
		{
			if (PS == CachedPlayerState)
			{
				// 플레이어 본인 위젯이면 확대, HPBar와 PlayerName 색 강조
				ExpandRenderSize();
				CircularHPBar->SetColorAndOpacity(FLinearColor(0.07f, 1.f, 0.1f, 1.f));
				PlayerName->SetColorAndOpacity(FLinearColor(1.f, 0.8f, 0.05f, 1.f));
			}
			else
			{
				// 타 플레이어 위젯이면 HPBar를 빨간색으로 색 강조
				CircularHPBar->SetColorAndOpacity(FLinearColor(1.f, 0.f, 0.02f, 1.f));
			}
		}
	}

	// 플레이어 연승 기록, 어트리뷰트 (HP) 구독
	CachedPlayerState->OnWinningStreakUpdated.AddUObject(this, &UPCPlayerRowWidget::SetWinningStreak);
	if (auto ASC = CachedPlayerState->GetAbilitySystemComponent())
	{
		if (auto AttributeSet = CachedPlayerState->GetAttributeSet())
		{
			ASC->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetPlayerHPAttribute())
			.AddUObject(this, &UPCPlayerRowWidget::UpdatePlayerHP);
		}
	}

	// 플레이어 이름 세팅
	auto NameText = FString::Printf(TEXT("%s"), *CachedPlayerState->LocalUserId);
	PlayerName->SetText(FText::FromString(NameText));

	if (auto AttributeSet = CachedPlayerState->GetAttributeSet())
	{
		// 플레이어 체력바 세팅
		auto HP = AttributeSet->GetPlayerHP();
		auto HPPercent = HP / 100.f;
		SetHP(HPPercent);

		auto HPText = FString::Printf(TEXT("%d"), static_cast<int32>(HP));
		PlayerHP->SetText(FText::FromString(HPText));
	}

	FGameplayTag CharacterTag;
	if (auto ASC = CachedPlayerState->GetAbilitySystemComponent())
	{
		FGameplayTagContainer Tags;
		ASC->GetOwnedGameplayTags(Tags);

		for (const FGameplayTag& Tag : Tags)
		{
			if (Tag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("Player.Type"))))
			{
				CharacterTag = Tag;
			}
		}
	}
	
	// 플레이어 초상화 세팅
	auto PortraitSoftPtr = PlayerPortrait->GetPlayerPortrait(CharacterTag);
	FSoftObjectPath TexturePath = PortraitSoftPtr.ToSoftObjectPath();
	FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
	TWeakObjectPtr<UPCPlayerRowWidget> WeakThis = this;
	
	Streamable.RequestAsyncLoad(TexturePath, [WeakThis, TexturePath]()
	{
		if (UTexture2D* Texture = Cast<UTexture2D>(TexturePath.ResolveObject()))
		{
			WeakThis->Img_Portrait->SetBrushFromTexture(Texture);
		}
	});
}

void UPCPlayerRowWidget::UnBindFromPlayerState()
{
	if (CachedPlayerState)
	{
		CachedPlayerState->OnWinningStreakUpdated.RemoveAll(this);

		if (auto ASC = CachedPlayerState->GetAbilitySystemComponent())
		{
			if (auto AttributeSet = CachedPlayerState->GetAttributeSet())
			{
				ASC->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetPlayerHPAttribute())
					.RemoveAll(this);
			}
		}

		CachedPlayerState = nullptr;
	}
}

void UPCPlayerRowWidget::UpdatePlayerHP(const FOnAttributeChangeData& Data)
{
	auto HP = Data.NewValue;
	auto HPPercent = HP / 100.f;
	SetHP(HPPercent);

	auto HPText = FString::Printf(TEXT("%d"), static_cast<int32>(HP));
	PlayerHP->SetText(FText::FromString(HPText));

	if (HP <= 0)
	{
		if (Img_Portrait)
		{
			Img_Portrait->SetColorAndOpacity(FLinearColor(0.1f, 0.1f, 0.1f, 1.0f));
		}
	}
}

void UPCPlayerRowWidget::SetWinningStreak(int32 NewWinningStreak)
{
	WinningStreak = NewWinningStreak;	
}

void UPCPlayerRowWidget::ExpandRenderSize()
{
	SetRenderScale(FVector2D(1.15f, 1.15f));
	SetRenderTranslation(FVector2D(-15.f, 0.f));
}

void UPCPlayerRowWidget::RestoreRenderSize()
{
	SetRenderScale(FVector2D(1.f, 1.f));
	SetRenderTranslation(FVector2D(0.f, 0.f));
}

void UPCPlayerRowWidget::PlayerPatrol()
{
	if (!CachedPlayerState) return;

	if (auto OwnerPC = Cast<APCCombatPlayerController>(GetOwningPlayer()))
	{
		OwnerPC->PlayerPatrol(CachedPlayerState);
	}
}

void UPCPlayerRowWidget::SetHP_Implementation(float HPPercent)
{
}

void UPCPlayerRowWidget::SetWinningFlame_Implementation()
{
}
