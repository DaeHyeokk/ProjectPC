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
	Btn_CameraSwitch->OnClicked.AddDynamic(this, &UPCPlayerRowWidget::SwitchCamera);

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
				ExpandRenderSize();
				CircularHPBar->SetColorAndOpacity(FLinearColor(0.07f, 1.f, 0.1f, 1.f));
				PlayerName->SetColorAndOpacity(FLinearColor(1.f, 0.8f, 0.05f, 1.f));
			}
			else
			{
				CircularHPBar->SetColorAndOpacity(FLinearColor(1.f, 0.f, 0.02f, 1.f));
			}
		}
	}

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

void UPCPlayerRowWidget::UpdatePlayerHP(const FOnAttributeChangeData& Data)
{
	// 플레이어 체력바 세팅
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

void UPCPlayerRowWidget::SwitchCamera()
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
