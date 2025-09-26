// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFramework/HelpActor/PCCarouselRing.h"

#include "BaseGameplayTags.h"
#include "Camera/CameraComponent.h"
#include "Character/Unit/PCBaseUnitCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/RotatingMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/GameState/PCCombatGameState.h"
#include "GameFramework/WorldSubsystem/PCUnitSpawnSubsystem.h"
#include "Kismet/KismetMathLibrary.h"
#include "Shop/PCShopManager.h"


APCCarouselRing::APCCarouselRing()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	PlayerRingRoot = CreateDefaultSubobject<USceneComponent>(TEXT("PlayerRingRoot"));
	PlayerRingRoot->SetupAttachment(SceneRoot);

	UnitRingRoot = CreateDefaultSubobject<USceneComponent>(TEXT("UnitRingRoot"));
	UnitRingRoot->SetupAttachment(SceneRoot);

	GateRoot = CreateDefaultSubobject<USceneComponent>(TEXT("GateRoot"));
	GateRoot->SetupAttachment(SceneRoot);

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(SceneRoot);
	SpringArm->SetRelativeLocation(CameraArmLocalLocation);
	SpringArm->SetRelativeRotation(CameraArmLocalRotation);
	SpringArm->TargetArmLength = CameraArmLength;
	
	SpringArm->bUsePawnControlRotation = false;
	SpringArm->bInheritPitch = false;
	SpringArm->bInheritRoll = false;
	SpringArm->bInheritYaw = false;

	CarouselCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("CarouselCamera"));
	CarouselCamera->SetupAttachment(SpringArm);
	CarouselCamera->FieldOfView = CameraFov;

	RotatingMovement = CreateDefaultSubobject<URotatingMovementComponent>(TEXT("RotatingMovement"));
	RotatingMovement->SetUpdatedComponent(UnitRingRoot);
	RotatingMovement->RotationRate = FRotator(0.f, UnitRingRotationRateYawDeg,0.f);
	RotatingMovement->bAutoActivate = false;
}

void APCCarouselRing::BeginPlay()
{
	Super::BeginPlay();
	if (CarouselCamera)
		CarouselCamera->Activate();
	
	if (bUnitRingRotate)
	{
		SetRotationOnActive(true);
	}

	BuildGates();
}

#if WITH_EDITOR
void APCCarouselRing::OnConstruction(const FTransform& transform)
{
	Super::OnConstruction(transform);

	if (!bDrawDebug) return;

	// 바깥/안쪽 링 디버그 원
	const FVector COuter = GetRingCenterWorld(PlayerRingRoot);
	const FVector CInner = GetRingCenterWorld(UnitRingRoot);

	DrawDebugCircle(GetWorld(), COuter, PlayerRingRadius, 64, DebugColorOuter, false, 0.f, 0, 2.f, FVector(1,0,0), FVector(0,1,0), false);
	DrawDebugCircle(GetWorld(), CInner, UnitRingRadius, 64, DebugColorInner, false, 0.f, 0, 2.f, FVector(1,0,0), FVector(0,1,0), false);

	// 게이트 박스 미리 보기
	for (UBoxComponent* B : GateBoxes)
	{
		if (!B) continue;
		const FTransform T = B->GetComponentTransform();
		DrawDebugBox(GetWorld(), T.GetLocation(), B->GetUnscaledBoxExtent(), T.GetRotation(), DebugColorGate, false, 0.f, 0, 1.5f);
	}
}
#endif

FVector APCCarouselRing::GetRingCenterWorld(const USceneComponent* Root) const
{
	check(Root);
	return Root->GetComponentTransform().TransformPosition(FVector(0,0,0));
}

FRotator APCCarouselRing::MakeFacingRotToCenter(const FVector& Pos, float ExtraYaw) const
{
	const FVector Center = GetActorLocation();
	const FRotator ToCenter = UKismetMathLibrary::FindLookAtRotation(Pos, Center);
	FRotator Rotator = ToCenter;
	Rotator.Yaw = FMath::UnwindDegrees(Rotator.Yaw + ExtraYaw);
	Rotator.Roll = 0.f;
	return Rotator;
}

float APCCarouselRing::GetPlayerSeatAngelDeg(int32 SeatIndex) const
{
	if (PlayerNumSlots <= 0)
		return PlayerRingStartAngleDeg;
	const float Step = 360.f / PlayerNumSlots;
	return PlayerRingStartAngleDeg + Step * SeatIndex;
}

FTransform APCCarouselRing::GetPlayerSlotTransformWorld(int32 Index) const
{
	const float Angle = GetPlayerSeatAngelDeg(Index);
	const FVector Center = GetRingCenterWorld(PlayerRingRoot);
	const FVector Dir = FRotator(0.f, Angle, 0.f).Vector();
	const FVector Pos = Center + Dir * PlayerRingRadius + FVector(0,0,PlayerRingHeight);

	FRotator Rotator = FRotator::ZeroRotator;
	Rotator = FRotator(0.f, Angle, 0.f);

	return FTransform(Rotator,Pos,FVector::OneVector);
}

FTransform APCCarouselRing::GetUnitSlotTransformWorld(int32 Index) const
{
	const float Step = (UnitRingNumSlots > 0) ? 360.f / UnitRingNumSlots : 360.f;
	const float Angle = UnitRingRotationRateYawDeg + Step * Index;
	const FVector Center = GetRingCenterWorld(UnitRingRoot);
	const FVector Dir = FRotator(0.f, Angle, 0.f).Vector();
	const FVector Pos = Center + Dir * UnitRingRadius + FVector(0,0,UnitRingHeight);
	const FRotator Rot = MakeFacingRotToCenter(Pos,0.f);
	return FTransform(Rot,Pos, FVector::ZeroVector);
}

void APCCarouselRing::ClearPickups()
{
	for (auto& W : SpawnedPickups)
	{
		if (APCBaseUnitCharacter* Unit = W.Get())
		{
			Unit->Destroy();
		}
	}
	SpawnedPickups.Reset();
}

void APCCarouselRing::SpawnPickups()
{
	ClearPickups();

	TArray<FGameplayTag> SpawnTag;

	if ( APCCombatGameState* PCGameState = GetWorld()->GetGameState<APCCombatGameState>())
	{
		auto ShopManager = PCGameState->GetShopManager();
		SpawnTag = ShopManager->GetCarouselUnitTags(1);
	}

	// 부모(부착 대상) 스케일이 0이면 자식 월드스케일도 0 됩니다.
	UnitRingRoot->SetWorldScale3D(FVector(1.f));
	UnitRingRoot->SetRelativeScale3D(FVector(1.f));

	const int32 Count = FMath::Max(0, NumPickupsToSpawn);
	for (int32 i = 0; i < SpawnTag.Num(); ++i)
	{
		FTransform Slot = GetUnitSlotTransformWorld(i);
		FTransform T = Slot;
		T.AddToTranslation(PickupLocalOffset);
		
		if (UPCUnitSpawnSubsystem* SpawnSystem = GetWorld()->GetSubsystem<UPCUnitSpawnSubsystem>())
		{
			if (APCBaseUnitCharacter* Unit = SpawnSystem->SpawnUnitByTag(SpawnTag[i]))
				{
					if (Unit)
					{
						// FinishSpawning 내부에서 완료됨(Subsystem 코드)
						Unit->SetActorHiddenInGame(false);

						// 월드 트랜스폼 먼저
						Unit->SetActorTransform(T, false, nullptr, ETeleportType::TeleportPhysics);

						// 컴포넌트 스케일/가시성 보정
						if (UCapsuleComponent* Cap = Unit->GetCapsuleComponent())
						{
							Cap->SetWorldScale3D(FVector(1.f));
						}
						if (USkeletalMeshComponent* SK = Unit->GetMesh())
						{
							SK->SetWorldScale3D(FVector(1.f));
							SK->SetHiddenInGame(false);
						}

						// 부착
						Unit->AttachToComponent(UnitRingRoot, FAttachmentTransformRules::KeepWorldTransform);

						// 최종 로그
						UE_LOG(LogTemp, Warning, TEXT("RingRoot=%s  Unit=%s  Mesh=%s"),
							*UnitRingRoot->GetComponentTransform().GetScale3D().ToString(),
							*Unit->GetActorScale3D().ToString(),
							Unit->GetMesh()? *Unit->GetMesh()->GetComponentScale().ToString() : TEXT("NoMesh"));
					}
				}
		}
	}
}


void APCCarouselRing::SetRotationOnActive(bool bOn)
{
	if (!RotatingMovement)
		return;
	RotatingMovement->RotationRate = FRotator(0.f, UnitRingRotationRateYawDeg,0.f);
	RotatingMovement->SetActive(bOn, false);
}


void APCCarouselRing::BuildGates()
{
	for (UBoxComponent* GateBox : GateBoxes)
	{
		if (GateBox)
		{
			GateBox->DestroyComponent();
		}
	}
	GateBoxes.Reset();

	if (PlayerNumSlots <= 0 )
		return;

	for (int32 i = 0; i < PlayerNumSlots; ++i)
	{
		// 각 Seat의 전면을 막는 박스
		const float CenterAngle = GetPlayerSeatAngelDeg(i);
		const float HalfArc = GateArcDeg * 0.5f;

		// 박스의 중심 각도
		const FVector CenterWorld = GetRingCenterWorld(PlayerRingRoot);
		const FVector Dir = FRotator(0.f, CenterAngle, 0.f).Vector();

		// 박스를 플레이어보다 약간 앞 으로 두어 이동 차단
		const float Radial = PlayerRingRadius - GateThickness * 0.5f;
		const FVector Pos = CenterWorld + Dir * Radial + FVector(0,0,PlayerRingHeight + GateHeight*0.5f);

		// 상자 크기
		const float ArcLen = 2.f * Radial * FMath::Sin(FMath::DegreesToRadians(HalfArc));
		const FVector BoxExtent(ArcLen, GateThickness * 0.5f, GateHeight*0.5);

		UBoxComponent* Box = NewObject<UBoxComponent>(this);
		Box->RegisterComponent();
		Box->AttachToComponent(GateRoot, FAttachmentTransformRules::KeepWorldTransform);
		Box->SetBoxExtent(BoxExtent, true);
		Box->SetCollisionProfileName(GateCollisionProfile);
		Box->SetHiddenInGame(true);

		FRotator Rotator(0.f, CenterAngle, 0.f);
		Box->SetWorldLocationAndRotation(Pos,Rotator);

		GateBoxes.Add(Box);
	}
}

void APCCarouselRing::OpenGateForSeat(int32 SeatIndex, bool bOpen)
{
	if (!GateBoxes.IsValidIndex(SeatIndex))
		return;
	UBoxComponent* Box = GateBoxes[SeatIndex];
	if (!Box)
		return;
	if (bOpen)
	{
		Box->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	else
	{
		Box->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}
}

void APCCarouselRing::OpenAllGates(bool bOpen)
{
	for (int32 i = 0; i < GateBoxes.Num(); ++i)
	{
		OpenGateForSeat(i, bOpen);
	}
}

void APCCarouselRing::ApplyCentralViewForSeat(APlayerController* PC, int32 SeatIndex, float BlendTime,
	float ExtraYawDeg)
{
	if (!PC || !SpringArm) return;

	const float Step = 360.f / 8.f;           // 45°
	const float Pitch = -60.f;                 // 원래 쓰던 값
	const float CamYaw =  SeatIndex * Step + ExtraYawDeg;

	SpringArm->TargetArmLength = 4000.f;       // 원래 쓰던 값
	SpringArm->SetRelativeRotation(FRotator(Pitch, CamYaw, 0.f));
	
}







