#include "WorldCCTVCameraDirector.h"

#include "Camera/CameraComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

AWorldCCTVCameraDirector::AWorldCCTVCameraDirector()
{
	PrimaryActorTick.bCanEverTick = true;

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	RootComponent = CameraComp;
}

void AWorldCCTVCameraDirector::BeginPlay()
{
	Super::BeginPlay();

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
}

void AWorldCCTVCameraDirector::RequestZoneTransition(const FWorldCameraZoneTransitionRequest& Request, AActor* NewTarget)
{
	if (bZoneTransitionInProgress)
	{
		return;
	}

	if (Request.Priority < CurrentPriority)
	{
		return;
	}

	PendingZoneRequest = Request;
	PendingTargetActor = NewTarget;
	CurrentPriority = Request.Priority;

	if (NewTarget)
	{
		SetTargetActor(NewTarget);
	}

	PendingLevelLoadNames.Reset();
	PendingLevelUnloadNames.Reset();

	TSet<FName> LoadNameSet;
	for (const TSoftObjectPtr<UWorld>& LevelAsset : Request.LevelsToLoad)
	{
		const FName LevelName = ResolveStreamLevelName(LevelAsset);
		if (!LevelName.IsNone())
		{
			LoadNameSet.Add(LevelName);
			PendingLevelLoadNames.AddUnique(LevelName);
		}
	}

	for (const TSoftObjectPtr<UWorld>& LevelAsset : Request.LevelsToUnload)
	{
		const FName LevelName = ResolveStreamLevelName(LevelAsset);
		if (!LevelName.IsNone() && !LoadNameSet.Contains(LevelName))
		{
			PendingLevelUnloadNames.AddUnique(LevelName);
		}
	}

	const bool bNeedsStreaming = (PendingLevelLoadNames.Num() > 0 || PendingLevelUnloadNames.Num() > 0);
	const bool bNeedsFadeFlow = Request.bUseFadeTransition || bNeedsStreaming || Request.bFreezePlayerDuringTransition;

	if (!bNeedsFadeFlow)
	{
		RequestAnchorTransform(Request.CameraAnchor, Request.AnchorBlendTime, Request.Priority);
		return;
	}

	bZoneTransitionInProgress = true;

	if (Request.bFreezePlayerDuringTransition)
	{
		StopTrackedPawnMovement();
		SetTransitionInputBlocked(true);
	}

	APlayerController* PC = ResolvePlayerController();
	if (Request.bUseFadeTransition && PC && PC->PlayerCameraManager && Request.FadeOutDuration > 0.f)
	{
		PC->PlayerCameraManager->StartCameraFade(0.f, 1.f, Request.FadeOutDuration, FLinearColor::Black, false, true);
		GetWorldTimerManager().SetTimer(
			TimerHandle_FadeOutFinished,
			this,
			&AWorldCCTVCameraDirector::HandleFadeOutFinished,
			Request.FadeOutDuration,
			false
		);
	}
	else
	{
		HandleFadeOutFinished();
	}
}

void AWorldCCTVCameraDirector::HandleFadeOutFinished()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_FadeOutFinished);

	if (PendingTargetActor.IsValid())
	{
		SetTargetActor(PendingTargetActor.Get());
	}

	if (PendingZoneRequest.bUseFadeTransition)
	{
		SnapToAnchorTransform(PendingZoneRequest.CameraAnchor, PendingZoneRequest.Priority);
	}
	else
	{
		RequestAnchorTransform(PendingZoneRequest.CameraAnchor, PendingZoneRequest.AnchorBlendTime, PendingZoneRequest.Priority);
	}

	BeginPendingLevelStreaming();
}

void AWorldCCTVCameraDirector::BeginPendingLevelStreaming()
{
	PendingStreamingIndex = 0;
	bStreamingUnloadPhase = (PendingLevelUnloadNames.Num() > 0);

	if (PendingLevelUnloadNames.Num() == 0 && PendingLevelLoadNames.Num() == 0)
	{
		FinishZoneTransition();
		return;
	}

	ProcessNextStreamingAction();
}

void AWorldCCTVCameraDirector::ProcessNextStreamingAction()
{
	if (bStreamingUnloadPhase)
	{
		if (PendingStreamingIndex >= PendingLevelUnloadNames.Num())
		{
			bStreamingUnloadPhase = false;
			PendingStreamingIndex = 0;
			ProcessNextStreamingAction();
			return;
		}

		const FName LevelName = PendingLevelUnloadNames[PendingStreamingIndex];
		FLatentActionInfo LatentInfo;
		LatentInfo.CallbackTarget = this;
		LatentInfo.ExecutionFunction = FName(TEXT("HandleStreamingStepFinished"));
		LatentInfo.Linkage = 0;
		LatentInfo.UUID = ++NextLatentUUID;

		UGameplayStatics::UnloadStreamLevel(this, LevelName, LatentInfo, PendingZoneRequest.bBlockOnLevelStreaming);
		return;
	}

	if (PendingStreamingIndex >= PendingLevelLoadNames.Num())
	{
		FinishZoneTransition();
		return;
	}

	const FName LevelName = PendingLevelLoadNames[PendingStreamingIndex];
	FLatentActionInfo LatentInfo;
	LatentInfo.CallbackTarget = this;
	LatentInfo.ExecutionFunction = FName(TEXT("HandleStreamingStepFinished"));
	LatentInfo.Linkage = 0;
	LatentInfo.UUID = ++NextLatentUUID;

	UGameplayStatics::LoadStreamLevel(this, LevelName, true, PendingZoneRequest.bBlockOnLevelStreaming, LatentInfo);
}

void AWorldCCTVCameraDirector::HandleStreamingStepFinished()
{
	++PendingStreamingIndex;
	ProcessNextStreamingAction();
}

void AWorldCCTVCameraDirector::FinishZoneTransition()
{
	APlayerController* PC = ResolvePlayerController();
	if (PendingZoneRequest.bUseFadeTransition && PC && PC->PlayerCameraManager && PendingZoneRequest.FadeInDuration > 0.f)
	{
		PC->PlayerCameraManager->StartCameraFade(1.f, 0.f, PendingZoneRequest.FadeInDuration, FLinearColor::Black, false, false);
	}

	if (PendingZoneRequest.bFreezePlayerDuringTransition)
	{
		SetTransitionInputBlocked(false);
	}

	bZoneTransitionInProgress = false;
	ClearPendingStreamingState();
}

void AWorldCCTVCameraDirector::ClearPendingStreamingState()
{
	PendingLevelLoadNames.Reset();
	PendingLevelUnloadNames.Reset();
	PendingStreamingIndex = 0;
	bStreamingUnloadPhase = true;
	PendingTargetActor.Reset();
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

APlayerController* AWorldCCTVCameraDirector::ResolvePlayerController() const
{
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
	{
		return PC;
	}

	if (APawn* Pawn = Cast<APawn>(TargetActor.Get()))
	{
		return Cast<APlayerController>(Pawn->GetController());
	}

	return nullptr;
}

void AWorldCCTVCameraDirector::SetTransitionInputBlocked(bool bBlocked)
{
	if (APlayerController* PC = ResolvePlayerController())
	{
		PC->SetIgnoreMoveInput(bBlocked);
		PC->SetIgnoreLookInput(bBlocked);

		if (bBlocked)
		{
			if (APawn* Pawn = PC->GetPawn())
			{
				if (UPawnMovementComponent* MoveComp = Pawn->GetMovementComponent())
				{
					MoveComp->StopMovementImmediately();
				}
			}
		}
	}
}

void AWorldCCTVCameraDirector::StopTrackedPawnMovement()
{
	APawn* Pawn = Cast<APawn>(PendingTargetActor.IsValid() ? PendingTargetActor.Get() : TargetActor.Get());
	if (!Pawn)
	{
		return;
	}

	if (UPawnMovementComponent* MoveComp = Pawn->GetMovementComponent())
	{
		MoveComp->StopMovementImmediately();
	}
}

FName AWorldCCTVCameraDirector::ResolveStreamLevelName(const TSoftObjectPtr<UWorld>& LevelAsset)
{
	if (LevelAsset.IsNull())
	{
		return NAME_None;
	}

	const FString LongPackageName = LevelAsset.ToSoftObjectPath().GetLongPackageName();
	return LongPackageName.IsEmpty() ? NAME_None : FName(*LongPackageName);
}
