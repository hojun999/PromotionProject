#pragma once

#include "CoreMinimal.h"
#include "UObject/SoftObjectPtr.h"
#include "GameFramework/Actor.h"
#include "WorldCCTVCameraDirector.generated.h"

class UCameraComponent;
class UWorld;

USTRUCT(BlueprintType)
struct FWorldCameraZoneTransitionRequest
{
	GENERATED_BODY()

	/** Camera anchor to move/snap to for this zone. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera")
	FTransform CameraAnchor = FTransform::Identity;

	/** Used only when not doing fade-based snap transitions. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera", meta=(ClampMin="0.0"))
	float AnchorBlendTime = 0.75f;

	/** Higher priority wins when overlapping zones try to request at the same time. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera")
	int32 Priority = 0;

	/** Sublevels to activate when entering this zone. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Streaming")
	TArray<TSoftObjectPtr<UWorld>> LevelsToLoad;

	/** Sublevels to deactivate when entering this zone. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Streaming")
	TArray<TSoftObjectPtr<UWorld>> LevelsToUnload;

	/** If true, do fade-out -> stream -> fade-in. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Transition")
	bool bUseFadeTransition = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Transition", meta=(EditCondition="bUseFadeTransition", ClampMin="0.0"))
	float FadeOutDuration = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Transition", meta=(EditCondition="bUseFadeTransition", ClampMin="0.0"))
	float FadeInDuration = 0.25f;

	/** If true, movement/look input is blocked during the transition and movement is stopped immediately. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Transition")
	bool bFreezePlayerDuringTransition = true;

	/** Uses blocking level streaming while screen is black. Safer for fixed-view swaps. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Streaming")
	bool bBlockOnLevelStreaming = true;
};

/**
 * Fixed-position CCTV-style world camera:
 * - Location is anchored (can transition between anchors)
 * - Rotation slowly tracks a target actor (player pawn)
 *
 * Place ONE in L_World and set the PlayerController view target to it.
 */
UCLASS()
class PCUBE_API AWorldCCTVCameraDirector : public AActor
{
	GENERATED_BODY()

public:
	AWorldCCTVCameraDirector();

	virtual void Tick(float DeltaSeconds) override;
	virtual void BeginPlay() override;

	/** Set who the camera should track (usually the player pawn). */
	UFUNCTION(BlueprintCallable, Category="World Camera")
	void SetTargetActor(AActor* NewTarget);

	/** Request camera anchor move. Rotation tracking continues automatically. */
	UFUNCTION(BlueprintCallable, Category="World Camera")
	void RequestAnchorTransform(const FTransform& NewAnchor, float BlendTime = 0.75f, int32 NewPriority = 0);

	/** Immediately snaps to anchor. */
	UFUNCTION(BlueprintCallable, Category="World Camera")
	void SnapToAnchorTransform(const FTransform& NewAnchor, int32 NewPriority = 0);

	/** Optional: override rotation interpolation speed at runtime. */
	UFUNCTION(BlueprintCallable, Category="World Camera")
	void SetRotationInterpSpeed(float NewSpeed);

	/** Zone-triggered view transition with optional fade and streaming. */
	UFUNCTION(BlueprintCallable, Category="World Camera")
	void RequestZoneTransition(const FWorldCameraZoneTransitionRequest& Request, AActor* NewTarget = nullptr);

	/** Read-only: current selected zone priority (prevents flicker when multiple triggers overlap). */
	UFUNCTION(BlueprintPure, Category="World Camera")
	int32 GetCurrentPriority() const { return CurrentPriority; }

	UFUNCTION(BlueprintPure, Category="World Camera")
	bool IsZoneTransitionInProgress() const { return bZoneTransitionInProgress; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UCameraComponent> CameraComp;
	
	/** Target to track (player). */
	UPROPERTY(Transient)
	TWeakObjectPtr<AActor> TargetActor;

	/** Look-at offset added to target's location (e.g., aim at chest/head). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tracking")
	FVector LookAtOffset = FVector(0.f, 0.f, 120.f);

	/** How fast the camera rotates towards target. Higher = snappier. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tracking", meta=(ClampMin="0.0"))
	float RotationInterpSpeed = 2.0f;

	/** If true, only yaw rotates; pitch is fixed (CCTV vibe). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tracking")
	bool bYawOnly = true;

	/** Used when bYawOnly=true. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tracking", meta=(EditCondition="bYawOnly"))
	float FixedPitch = -10.0f;

	/** Clamp pitch (when bYawOnly=false) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tracking", meta=(EditCondition="!bYawOnly"))
	bool bClampPitch = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tracking", meta=(EditCondition="!bYawOnly"))
	float MinPitch = -35.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tracking", meta=(EditCondition="!bYawOnly"))
	float MaxPitch = 15.0f;

	/** Smooth location transition settings */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Anchor")
	bool bEaseInOut = true;

private:
	// Anchor transition state
	bool bTransitioning = false;
	float TransitionElapsed = 0.f;
	float TransitionDuration = 0.f;
	FTransform TransitionStart;
	FTransform TransitionTarget;

	// Priority-based zone selection (avoid flicker)
	int32 CurrentPriority = 0;

	// Zone transition orchestration
	bool bZoneTransitionInProgress = false;
	FWorldCameraZoneTransitionRequest PendingZoneRequest;
	TWeakObjectPtr<AActor> PendingTargetActor;
	TArray<FName> PendingLevelLoadNames;
	TArray<FName> PendingLevelUnloadNames;
	bool bStreamingUnloadPhase = true;
	int32 PendingStreamingIndex = 0;
	int32 NextLatentUUID = 1000;
	FTimerHandle TimerHandle_FadeOutFinished;

	FRotator ComputeDesiredRotation() const;
	void UpdateAnchorTransition(float DeltaSeconds);

	APlayerController* ResolvePlayerController() const;
	void SetTransitionInputBlocked(bool bBlocked);
	void StopTrackedPawnMovement();
	void BeginPendingLevelStreaming();
	void ProcessNextStreamingAction();
	void FinishZoneTransition();
	void ClearPendingStreamingState();
	static FName ResolveStreamLevelName(const TSoftObjectPtr<UWorld>& LevelAsset);

	UFUNCTION()
	void HandleFadeOutFinished();

	UFUNCTION()
	void HandleStreamingStepFinished();
};
