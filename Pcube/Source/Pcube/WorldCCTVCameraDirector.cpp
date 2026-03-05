#include "WorldCCTVCameraDirector.h"

#include "Camera/CameraComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

AWorldCCTVCameraDirector::AWorldCCTVCameraDirector()
{
	PrimaryActorTick.bCanEverTick = true;

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	RootComponent = CameraComp;

	// AActor default is hidden in game false; camera component renders anyway when view-targeted.
}

void AWorldCCTVCameraDirector::BeginPlay()
{
	Super::BeginPlay();

	// Optional convenience: auto-target player pawn if not set.
	if (!TargetActor.IsValid())
	{
		if (APawn* Pawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
		{
			TargetActor = Pawn;
		}
	}
}

void AWorldCCTVCameraDirector::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateAnchorTransition(DeltaSeconds);

	// Rotate to track target.
	if (TargetActor.IsValid())
	{
		const FRotator Desired = ComputeDesiredRotation();
		const FRotator NewRot = FMath::RInterpTo(GetActorRotation(), Desired, DeltaSeconds, RotationInterpSpeed);
		SetActorRotation(NewRot);
	}
}

void AWorldCCTVCameraDirector::SetTargetActor(AActor* NewTarget)
{
	TargetActor = NewTarget;
}

void AWorldCCTVCameraDirector::SetRotationInterpSpeed(float NewSpeed)
{
	RotationInterpSpeed = FMath::Max(0.f, NewSpeed);
}

void AWorldCCTVCameraDirector::RequestAnchorTransform(const FTransform& NewAnchor, float BlendTime, int32 NewPriority)
{
	// Avoid lower priority triggers overriding a higher-priority camera.
	if (NewPriority < CurrentPriority)
	{
		return;
	}

	CurrentPriority = NewPriority;

	if (BlendTime <= 0.f)
	{
		SnapToAnchorTransform(NewAnchor, NewPriority);
		return;
	}

	bTransitioning = true;
	TransitionElapsed = 0.f;
	TransitionDuration = BlendTime;
	TransitionStart = GetActorTransform();
	TransitionTarget = NewAnchor;
}

void AWorldCCTVCameraDirector::SnapToAnchorTransform(const FTransform& NewAnchor, int32 NewPriority)
{
	CurrentPriority = NewPriority;
	bTransitioning = false;
	TransitionElapsed = 0.f;
	TransitionDuration = 0.f;

	SetActorLocation(NewAnchor.GetLocation());
	// Rotation is driven by tracking; we don't apply anchor rotation here on purpose.
}

void AWorldCCTVCameraDirector::UpdateAnchorTransition(float DeltaSeconds)
{
	if (!bTransitioning)
	{
		return;
	}

	TransitionElapsed += DeltaSeconds;
	const float AlphaRaw = (TransitionDuration > 0.f) ? (TransitionElapsed / TransitionDuration) : 1.f;
	const float Alpha = FMath::Clamp(AlphaRaw, 0.f, 1.f);

	float T = Alpha;
	if (bEaseInOut)
	{
		// SmoothStep: 3t^2 - 2t^3
		T = (Alpha * Alpha) * (3.f - 2.f * Alpha);
	}

	const FVector StartLoc = TransitionStart.GetLocation();
	const FVector TargetLoc = TransitionTarget.GetLocation();
	const FVector NewLoc = FMath::Lerp(StartLoc, TargetLoc, T);
	SetActorLocation(NewLoc);

	if (Alpha >= 1.f)
	{
		bTransitioning = false;
	}
}

FRotator AWorldCCTVCameraDirector::ComputeDesiredRotation() const
{
	if (!TargetActor.IsValid())
	{
		return GetActorRotation();
	}

	const FVector CamLoc = GetActorLocation();
	const FVector TargetLoc = TargetActor->GetActorLocation() + LookAtOffset;

	FRotator LookAt = UKismetMathLibrary::FindLookAtRotation(CamLoc, TargetLoc);

	if (bYawOnly)
	{
		LookAt.Pitch = FixedPitch;
		LookAt.Roll = 0.f;
	}
	else
	{
		if (bClampPitch)
		{
			LookAt.Pitch = FMath::Clamp(LookAt.Pitch, MinPitch, MaxPitch);
		}
		LookAt.Roll = 0.f;
	}

	return LookAt;
}