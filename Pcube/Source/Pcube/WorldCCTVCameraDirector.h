#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WorldCCTVCameraDirector.generated.h"

class UCameraComponent;

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

	/** Read-only: current selected zone priority (prevents flicker when multiple triggers overlap). */
	UFUNCTION(BlueprintPure, Category="World Camera")
	int32 GetCurrentPriority() const { return CurrentPriority; }

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

	FRotator ComputeDesiredRotation() const;
	void UpdateAnchorTransition(float DeltaSeconds);
};