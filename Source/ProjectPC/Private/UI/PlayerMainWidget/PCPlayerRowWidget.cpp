// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PlayerMainWidget/PCPlayerRowWidget.h"

#include "AbilitySystemComponent.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"

#include "AbilitySystem/Player/AttributeSet/PCPlayerAttributeSet.h"
#include "GameFramework/PlayerState/PCPlayerState.h"


bool UPCPlayerRowWidget::Initialize()
{
	if (!Super::Initialize())
		return false;

	if (!Btn_CameraSwitch) return false;
	Btn_CameraSwitch->OnClicked.AddDynamic(this, &UPCPlayerRowWidget::SwitchCamera);

	return true;
}

// void UPCPlayerRowWidget::SetupPlayerInfo(const FString& NewPlayerName, float NewPlayerHP, FGameplayTag NewPlayerCharacterTag)
// {
// 	if (!PlayerName || !PlayerHP || !CircularHPBar || !Img_Portrait) return; 
//
// 	// 플레이어 이름 세팅
// 	auto NameText = FString::Printf(TEXT("%s"), *NewPlayerName);
// 	PlayerName->SetText(FText::FromString(NameText));
//
// 	// 플레이어 체력바 세팅
// 	auto HP = NewPlayerHP;
// 	auto HPPercent = HP / 100.f;
// 	SetHP(HPPercent);
//
// 	auto HPText = FString::Printf(TEXT("%d"), static_cast<int32>(HP));
// 	PlayerHP->SetText(FText::FromString(HPText));
//
// 	// 플레이어 초상화 세팅
// 	auto PortraitSoftPtr = PlayerPortrait->GetPlayerPortrait(NewPlayerCharacterTag);
// 	FSoftObjectPath TexturePath = PortraitSoftPtr.ToSoftObjectPath();
// 	FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
// 	
// 	Streamable.RequestAsyncLoad(TexturePath, [this, TexturePath]()
// 	{
// 		if (UTexture2D* Texture = Cast<UTexture2D>(TexturePath.ResolveObject()))
// 		{
// 			Img_Portrait->SetBrushFromTexture(Texture);
// 		}
// 	});
// }

void UPCPlayerRowWidget::SetupPlayerInfo(APCPlayerState* NewPlayerState)
{
	if (!NewPlayerState) return;
	if (!PlayerName || !PlayerHP || !CircularHPBar || !Img_Portrait) return;

	CachedPlayerState = NewPlayerState;

	CachedPlayerState->OnWinningStreakUpdated.AddUObject(this, &UPCPlayerRowWidget::SetWinningFlame);

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

void UPCPlayerRowWidget::UpdatePlayerHP()
{
	if (!CachedPlayerState) return;
	if (auto AttributeSet = CachedPlayerState->GetAttributeSet())
	{
		// 플레이어 체력바 세팅
		auto HP = AttributeSet->GetPlayerHP();
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
}

// void UPCPlayerRowWidget::UpdatePlayerHP(float NewPlayerHP)
// {
// 	if (!PlayerHP || !CircularHPBar) return;
// 	
// 	auto HP = NewPlayerHP;
// 	auto HPPercent = HP / 100.f;
// 	SetHP(HPPercent);
//
// 	auto HPText = FString::Printf(TEXT("%d"), static_cast<int32>(HP));
// 	PlayerHP->SetText(FText::FromString(HPText));
//
// 	if (HP <= 0)
// 	{
// 		if (Img_Portrait)
// 		{
// 			Img_Portrait->SetColorAndOpacity(FLinearColor(0.1f, 0.1f, 0.1f, 1.0f));
// 		}
// 	}
// }

void UPCPlayerRowWidget::SwitchCamera()
{
	if (!CachedPlayerState) return;

	UE_LOG(LogTemp, Error, TEXT("Switch to : %s"), *CachedPlayerState->LocalUserId);
}

void UPCPlayerRowWidget::SetHP_Implementation(float HPPercent)
{
}

void UPCPlayerRowWidget::SetWinningFlame_Implementation()
{
	if (!CachedPlayerState) return;

	if (CachedPlayerState->GetPlayerWinningStreak() >= 3)
	{
		bIsBurning = true;
	}
	else
	{
		bIsBurning = false;
	}
}
