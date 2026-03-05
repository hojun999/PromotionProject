#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WorldCameraZoneTrigger.generated.h"

class UBoxComponent;
class AWorldCCTVCameraDirector;

/**
 * Place in L_World to switch the CCTV camera to a new anchor when player enters.
 * Uses priority to avoid flicker with overlapping zones.
 */
UCLASS()
class PCUBE_API AWorldCameraZoneTrigger : public AActor
{
	GENERATED_BODY()

public:
	AWorldCameraZoneTrigger();

	virtual void BeginPlay() override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UBoxComponent> TriggerBox;

	/** Optional explicit director reference. If null, it will auto-find the first one in the level. */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Camera")
	TObjectPtr<AWorldCCTVCameraDirector> Director;

	/** Anchor location/transform for the camera when in this zone. Rotation is ignored (tracking drives rotation). */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Camera")
	FTransform CameraAnchor;

	/** Blend time when switching to this zone. */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Camera", meta=(ClampMin="0.0"))
	float BlendTime = 0.75f;

	/** Higher priority wins when zones overlap. */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Camera")
	int32 Priority = 0;

	/** If true, also updates director rotation speed on enter (useful per-area). */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Camera")
	bool bOverrideRotationSpeed = false;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Camera", meta=(EditCondition="bOverrideRotationSpeed", ClampMin="0.0"))
	float RotationInterpSpeedOverride = 2.0f;

	/** If true, we ignore end overlap (camera stays until another zone sets it). */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Camera")
	bool bIgnoreEndOverlap = true;

	UFUNCTION()
	void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
	bool IsPlayerActor(AActor* Actor) const;
	void EnsureDirector();
};