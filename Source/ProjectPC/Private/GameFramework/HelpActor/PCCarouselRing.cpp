// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFramework/HelpActor/PCCarouselRing.h"

#include "BaseGameplayTags.h"
#include "Camera/CameraComponent.h"
#include "Character/Player/PCPlayerCharacter.h"
#include "Character/Unit/PCBaseUnitCharacter.h"
#include "Character/Unit/PCCarouselHeroCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/RotatingMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/GameState/PCCombatGameState.h"
#include "GameFramework/WorldSubsystem/PCItemManagerSubsystem.h"
#include "GameFramework/WorldSubsystem/PCUnitSpawnSubsystem.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Shop/PCShopManager.h"


APCCarouselRing::APCCarouselRing()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	
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

	CarouselCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("CarouselCamera"));
	CarouselCamera->SetupAttachment(SpringArm);
	CarouselCamera->FieldOfView = CameraFov;

	RotatingMovement = CreateDefaultSubobject<URotatingMovementComponent>(TEXT("RotatingMovement"));
	RotatingMovement->SetUpdatedComponent(UnitRingRoot);
	RotatingMovement->RotationRate = FRotator(0.f, UnitRingRotationRateYawDeg,0.f);
	RotatingMovement->bAutoActivate = false;
}

void APCCarouselRing::Server_TryPickForPlayer_Implementation(APCPlayerCharacter* Picker)
{
	if (!HasAuthority() || !Picker) return;

	const int32 Seat = GetSeatOfPlayer(Picker);
	if (Seat == INDEX_NONE || SeatToUnit.Contains(Seat)) return;

	// 각도 → 슬롯
	const FVector CenterWS = GetActorLocation();
	const FVector ToP = (Picker->GetActorLocation() - CenterWS);
	float PlayerAngle = FMath::RadiansToDegrees(FMath::Atan2(ToP.Y, ToP.X));
	PlayerAngle = FMath::UnwindDegrees(PlayerAngle);

	const int32 prefIdx = ComputeSlotIndexForAngle(PlayerAngle);
	const int32 slotIdx = ChooseIdealFreeSlotNear(prefIdx);
	if (slotIdx == INDEX_NONE) return;

	if (APCCarouselHeroCharacter* Target = ResolveUnitBySlot(slotIdx))
	{
		if (!Target->IsPicked())
		{
			// 한 곳에서만 확정
			SeatToUnit.Add(Seat, Target);
			Target->MarkPicked();
			Target->Server_StartFollowing(Picker);
		}
	}
}

void APCCarouselRing::Server_StartCarousel_Implementation(float InStartAngleDeg, float InAngularSpeedDegPerSec)
{
	StartServerTime        = NowServer();
	StartAngleDeg          = InStartAngleDeg;
	AngularSpeedDegPerSec  = InAngularSpeedDegPerSec;
	SeatToUnit.Reset();

	// 시각 회전도 동일 파라미터로 구동
	if (RotatingMovement)
	{
		RotatingMovement->RotationRate = FRotator(0.f, AngularSpeedDegPerSec, 0.f);
		RotatingMovement->SetActive(true);
	}
	UnitRingRoot->SetRelativeRotation(FRotator(0.f, StartAngleDeg, 0.f));

	// 클라에 동기화
	Multicast_StartCarouselRotation(true, StartAngleDeg, AngularSpeedDegPerSec);

	// 얼굴방향 보정 타이머(60Hz)
	if (!GetWorldTimerManager().IsTimerActive(CarouselFacingTimer))
	{
		GetWorldTimerManager().SetTimer(CarouselFacingTimer, this, &APCCarouselRing::TickFaceAlongOrbit, 0.016f, true);
	}
}

void APCCarouselRing::Server_FinishCarousel_Implementation()
{
	if (!HasAuthority()) return;

	if (APCCombatGameState* GS = GetWorld()->GetGameState<APCCombatGameState>())
	{
		if (auto* Shop = GS->GetShopManager())
		{
			for (auto& WeakUnit : SpawnedPickups)
			{
				APCCarouselHeroCharacter* Unit = WeakUnit.Get();
				if (!Unit) continue;

				if (!Unit->IsPicked())
				{
					Shop->ReturnUnitToShopByTag(Unit->GetUnitTag());
					Unit->Destroy();
					continue;
				}
				Unit->Destroy();
			}
		}
	}
	SpawnedPickups.Reset();
	SeatToUnit.Reset();

	Multicast_StartCarouselRotation(false, StartAngleDeg, AngularSpeedDegPerSec);
}

int32 APCCarouselRing::GetSeatOfPlayer(const APCPlayerCharacter* Player) const
{
	if (APCPlayerState* PCPS = Player->GetPlayerState<APCPlayerState>())
	{
		return PCPS->SeatIndex;
	}

	return -1;
}

void APCCarouselRing::RegisterUnitAtIndex(int32 SlotIndex, APCCarouselHeroCharacter* CarouselUnit)
{
	if (!CarouselUnit) return;
	if (IndexToUnit.Num() != UnitRingNumSlots) IndexToUnit.SetNum(UnitRingNumSlots);
	IndexToUnit[ClampSlotIndex(SlotIndex)] = CarouselUnit;
	CarouselUnit->OwnerRing = this;
}

float APCCarouselRing::NowServer() const
{
	if (const AGameStateBase* GS = GetWorld()->GetGameState())
		return GS->GetServerWorldTimeSeconds();
	return GetWorld()->TimeSeconds; 
}

float APCCarouselRing::CurrentOrbitAngleDeg() const
{
	const float t = NowServer() - StartServerTime;
	return FMath::UnwindDegrees(StartAngleDeg + AngularSpeedDegPerSec * t);
}

int32 APCCarouselRing::ClampSlotIndex(int32 I) const
{
	if (UnitRingNumSlots <= 0) return 0;
	I %= UnitRingNumSlots;
	if (I < 0) I += UnitRingNumSlots;
	return I;
}

int32 APCCarouselRing::ChooseIdealFreeSlotNear(int32 PrefIdx) const
{
	// 선호 슬롯이 비었으면 반환, 아니면 양옆 탐색
	if (APCCarouselHeroCharacter* U = ResolveUnitBySlot(PrefIdx))
		if (!U->IsPicked()) return PrefIdx;

	const int32 R = UnitRingNumSlots / 2;
	for (int d=1; d<=R; ++d)
	{
		const int32 r = ClampSlotIndex(PrefIdx + d);
		if (APCCarouselHeroCharacter* Ur = ResolveUnitBySlot(r))
			if (!Ur->IsPicked()) return r;

		const int32 l = ClampSlotIndex(PrefIdx - d);
		if (APCCarouselHeroCharacter* Ul = ResolveUnitBySlot(l))
			if (!Ul->IsPicked()) return l;
	}
	return INDEX_NONE;
}

int32 APCCarouselRing::ComputeSlotIndexForAngle(float PlayerAngleDeg) const
{
	const float orbit = CurrentOrbitAngleDeg();
	const float step  = 360.f / FMath::Max(1, UnitRingNumSlots);
	const float rel   = FMath::UnwindDegrees(PlayerAngleDeg - orbit);
	return ClampSlotIndex(FMath::FloorToInt(rel / step + 0.5f)); // 반올림 버전
}

APCCarouselHeroCharacter* APCCarouselRing::ResolveUnitBySlot(int32 SlotIdx) const
{
	if (!IndexToUnit.IsValidIndex(SlotIdx)) return nullptr;
	return IndexToUnit[SlotIdx].Get();
}

void APCCarouselRing::CommitPick(APCPlayerCharacter* Picker, int32 Seat, APCCarouselHeroCharacter* Target)
{
	SeatToUnit.Add(Seat, Target);
	Target->MarkPicked();                  
	Target->Server_StartFollowing(Picker); 
}

void APCCarouselRing::TickFaceAlongOrbit()
{
	const FVector Center = GetRingCenterWorld(UnitRingRoot);

	for (auto& WeakUnit : SpawnedPickups)
	{
		APCCarouselHeroCharacter* Unit = WeakUnit.Get();
		if (!IsValid(Unit) || Unit->IsPicked()) continue;

		const FVector Loc = Unit->GetActorLocation();

		FVector Radial = (Loc - Center);
		Radial.Z = 0.f;
		if (!Radial.Normalize()) continue;

		FVector TangentCW(-Radial.Y, Radial.X, 0.f);

		FRotator FaceYaw = TangentCW.Rotation();
		FaceYaw.Yaw = FMath::UnwindDegrees(FaceYaw.Yaw + TangentYawOffsetDeg);
		FaceYaw.Pitch = 0.f;
		FaceYaw.Roll = 0.f;

		Unit->SetActorRotation(FaceYaw);
	}
}

void APCCarouselRing::Multicast_StartCarouselRotation_Implementation(bool bStart, float InStartAngleDeg, float InAngularSpeedDegPerSec)
{
	if (bStart)
	{
		StartAngleDeg         = InStartAngleDeg;
		AngularSpeedDegPerSec = InAngularSpeedDegPerSec;

		UnitRingRoot->SetRelativeRotation(FRotator(0.f, StartAngleDeg, 0.f));
		if (RotatingMovement)
		{
			RotatingMovement->RotationRate = FRotator(0.f, AngularSpeedDegPerSec, 0.f);
			RotatingMovement->SetActive(true);
		}
		if (!GetWorldTimerManager().IsTimerActive(CarouselFacingTimer))
		{
			GetWorldTimerManager().SetTimer(CarouselFacingTimer, this, &APCCarouselRing::TickFaceAlongOrbit, 0.016f, true);
		}
	}
	else
	{
		if (RotatingMovement) RotatingMovement->SetActive(false);
		GetWorldTimerManager().ClearTimer(CarouselFacingTimer);
	}
}

void APCCarouselRing::BeginPlay()
{
	Super::BeginPlay();

	IndexToUnit.SetNum(UnitRingNumSlots);
	
	if (CarouselCamera)
		CarouselCamera->Activate();

	if (bUnitRingRotate)
	{
		SetRotationOnActive(true);
		GetWorldTimerManager().SetTimer(CarouselFacingTimer, this, &APCCarouselRing::TickFaceAlongOrbit, 0.033f, true);
	}
	
	BuildGates();
}

void APCCarouselRing::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(APCCarouselRing, StartServerTime);
	DOREPLIFETIME(APCCarouselRing, StartAngleDeg);
	DOREPLIFETIME(APCCarouselRing, AngularSpeedDegPerSec);
	DOREPLIFETIME(APCCarouselRing, UnitRingNumSlots);
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
		if (APCCarouselHeroCharacter* Unit = W.Get())
		{
			Unit->Destroy();
		}
	}
	SpawnedPickups.Reset();
}

void APCCarouselRing::SpawnPickups(int32 Stage)
{
	ClearPickups();

	TArray<FGameplayTag> SpawnTag;

	if ( APCCombatGameState* PCGameState = GetWorld()->GetGameState<APCCombatGameState>())
	{
		auto ShopManager = PCGameState->GetShopManager();
		SpawnTag = ShopManager->GetCarouselUnitTags(Stage);
	}

	// 부모(부착 대상) 스케일이 0이면 자식 월드스케일도 0 됩니다.
	UnitRingRoot->SetWorldScale3D(FVector(1.f));
	UnitRingRoot->SetRelativeScale3D(FVector(1.f));

	for (int32 i = 0; i < SpawnTag.Num(); ++i)
	{
		FTransform Slot = GetUnitSlotTransformWorld(i);
		FTransform T = Slot;
		T.AddToTranslation(PickupLocalOffset);
		TArray<FGameplayTag> ItemTags;
		ItemTags.SetNum(SpawnTag.Num());
		
		if (UPCItemManagerSubsystem* ItemManager = GetWorld()->GetSubsystem<UPCItemManagerSubsystem>())
		{
			ItemTags[i] = ItemManager->GetRandomBaseItem();
		}
		
		if (UPCUnitSpawnSubsystem* SpawnSystem = GetWorld()->GetSubsystem<UPCUnitSpawnSubsystem>())
		{
			if (APCCarouselHeroCharacter* Unit = SpawnSystem->SpawnCarouselHeroByTag(SpawnTag[i], ItemTags[i]))
				{
					if (Unit)
					{
						Unit->OwnerRing = this;
						SpawnedPickups.AddUnique(Unit);
						RegisterUnitAtIndex(i,Unit);
						
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

						TickFaceAlongOrbit();
						
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

void APCCarouselRing::NotifyPicked(APCCarouselHeroCharacter* Unit, int32 Seat)
{
	if (!HasAuthority() || !Unit) return;

	if (SeatToUnit.Contains(Seat)) return;

	if (!SpawnedPickups.Contains(Unit)) return;

	SeatToUnit.Add(Seat, Unit);
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
	for (UStaticMeshComponent* GateMesh : GateMeshes)
	{
		if (GateMesh)
		{
			GateMesh->DestroyComponent();
		}
	}
	GateMeshes.Reset();

	if (PlayerNumSlots <= 0 )
		return;

	if (PlayerNumSlots <= 0 || !GateRoot || !GateStaticMesh)
		return;

	const FVector CenterWorld = GetRingCenterWorld(PlayerRingRoot);

	for (int32 i = 0; i < PlayerNumSlots; ++i)
	{
		const float CenterAngleDeg = GetPlayerSeatAngelDeg(i);
		const FVector Dir = FRotator(0.f, CenterAngleDeg, 0.f).Vector();
		const FVector Pos = CenterWorld + Dir * PlayerRingRadius + FVector(0,0, GateHeight);

		UStaticMeshComponent* Gate = NewObject<UStaticMeshComponent>(this);
		Gate->SetMobility(EComponentMobility::Movable);
		Gate->SetStaticMesh(GateStaticMesh);
		Gate->SetCollisionProfileName(GateCollisionProfile);
		Gate->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

		Gate->RegisterComponent();
		Gate->AttachToComponent(GateRoot, FAttachmentTransformRules::KeepWorldTransform);
		Gate->SetWorldLocation(Pos);

		GateMeshes.Add(Gate);
	}
}

void APCCarouselRing::OpenGateForSeat(int32 SeatIndex, bool bOpen)
{
	if (!GateMeshes.IsValidIndex(SeatIndex)) return;

	UStaticMeshComponent* Gate = GateMeshes[SeatIndex];

	if (!Gate) return;

	Gate->SetCollisionEnabled(bOpen ? ECollisionEnabled::NoCollision : ECollisionEnabled::QueryAndPhysics);
	Gate->SetHiddenInGame(bOpen);
}

void APCCarouselRing::OpenAllGates(bool bOpen)
{
	for (int32 i = 0; i < GateMeshes.Num(); ++i)
	{
		OpenGateForSeat(i, bOpen);
	}
}

void APCCarouselRing::Multicast_OpenAllGates_Implementation(bool bOpen)
{
	OpenAllGates(bOpen);
}

void APCCarouselRing::Multicast_SetGateOpen_Implementation(int32 SeatIndex, bool bOpen)
{
	OpenGateForSeat(SeatIndex, bOpen);
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







