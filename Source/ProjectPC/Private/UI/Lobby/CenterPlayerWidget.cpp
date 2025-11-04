// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Lobby/CenterPlayerWidget.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Engine/Texture2D.h"
#include "GameFramework/PlayerState/PCPlayerState.h"

FSlateBrush UCenterPlayerWidget::MakeBrush(UTexture2D* Tex, FVector2D Size, FMargin Margin, FLinearColor Tint)
{
    FSlateBrush B;
    B.SetResourceObject(Tex);
    if (Size != FVector2D::ZeroVector) B.ImageSize = Size;
    B.Margin = Margin;
    B.TintColor = Tint;
    return B;
}

void UCenterPlayerWidget::NativeConstruct()
{
    Super::NativeConstruct();
    ApplyStyleOnce();

    // 기본 표시
    if (TbSubtitle) TbSubtitle->SetText(FText::FromString(TEXT("Little Legend")));
    if (TbName)     TbName->SetText(FText::FromString(TEXT("Player")));
    if (Img_Ready) Img_Ready->SetVisibility(ESlateVisibility::Hidden);
    if (TbLevel)    TbLevel->SetText(FText::AsNumber(CachedLevel));
}

void UCenterPlayerWidget::ApplyStyleOnce()
{
    if (RootFrame)
    {
        if (FrameTexture) RootFrame->SetBrush(MakeBrush(FrameTexture, FVector2D::ZeroVector, FrameMargin, FrameTint));
        else              RootFrame->SetBrushColor(FLinearColor(0.f,0.f,0.f,0.35f));
        RootFrame->SetPadding(FMargin(16.f));
    }

    if (ImgCrown)
    {
        if (CrownTexture) ImgCrown->SetBrushFromTexture(CrownTexture);
        ImgCrown->SetVisibility(ESlateVisibility::Collapsed);
    }

    if (ImgAvatar)
    {
        if (DefaultAvatar) ImgAvatar->SetBrushFromTexture(DefaultAvatar);
        ImgAvatar->SetColorAndOpacity(FLinearColor::White);
    }

    if (LevelBadge)
    {
        LevelBadge->SetBrushColor(FLinearColor(0.f,0.f,0.f,0.55f));
    }

    if (Slot_LittleLegend)
    {
        // 빈 슬롯 느낌: 약간 어두운 사각형 + 외곽선
        Slot_LittleLegend->SetBrushColor(FLinearColor(0.f,0.f,0.f,0.35f));
        Slot_LittleLegend->SetPadding(FMargin(0.f));
    }
}

void UCenterPlayerWidget::SetPlayer(APCPlayerState* PS)
{
    PlayerPS = PS;
    if (!PS) return;
    
    SetDisplayName(FText::FromString(PS->LocalUserId));
    SetLeader(PS->bIsLeader);

    // 레벨/아바타 소스가 아직 없으면 임시 값
    SetLevel(PS->PlayerLevel > 0 ? PS->PlayerLevel : CachedLevel);

    if (Img_Ready)  Img_Ready->SetVisibility(PlayerPS->bIsReady  ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
}

void UCenterPlayerWidget::SetDisplayName(const FText& Name)
{
    
    if (TbName) TbName->SetText(Name);
}

void UCenterPlayerWidget::SetLevel(int32 Level)
{
    CachedLevel = FMath::Max(1, Level);
    if (TbLevel) TbLevel->SetText(FText::AsNumber(CachedLevel));
}

void UCenterPlayerWidget::SetLeader(bool bLeader)
{
    bIsLeader = bLeader;
    if (ImgCrown)
        ImgCrown->SetVisibility(bLeader ? ESlateVisibility::HitTestInvisible
                                        : ESlateVisibility::Collapsed);
}

void UCenterPlayerWidget::SetAvatar(UTexture2D* AvatarTex)
{
    CachedAvatar = AvatarTex ? AvatarTex : DefaultAvatar;
    if (ImgAvatar && CachedAvatar)
        ImgAvatar->SetBrushFromTexture(CachedAvatar);
}

void UCenterPlayerWidget::SetLittleLegendEmpty(bool bEmpty)
{
    if (!Slot_LittleLegend) return;
    // 나중에 실제 프리뷰 들어가면 여기서 위젯 교체
    Slot_LittleLegend->SetBrushColor(bEmpty ? FLinearColor(0.f,0.f,0.f,0.35f)
                                            : FLinearColor(1.f,1.f,1.f,0.02f));
}

