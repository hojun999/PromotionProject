#pragma once

#include "CoreMinimal.h"
#include "UObject/SoftObjectPtr.h"
#include "GameFramework/Actor.h"
#include "WorldCameraZoneTrigger.generated.h"

class UBoxComponent;
class UWorld;
class AWorldCCTVCameraDirector;

/**
 * 플레이어가 진입하면 CCTV 뷰로 전환
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

	/** Blend time when switching to this zone when no fade transition is used. */
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

	/** If true, use fade-out -> stream -> fade-in flow when entering this zone. */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Transition")
	bool bUseFadeTransition = false;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Transition", meta=(EditCondition="bUseFadeTransition", ClampMin="0.0"))
	float FadeOutDuration = 0.25f;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Transition", meta=(EditCondition="bUseFadeTransition", ClampMin="0.0"))
	float FadeInDuration = 0.25f;

	/** Player movement/look input is blocked during transition. */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Transition")
	bool bFreezePlayerDuringTransition = false;

	/** Block while streaming levels while screen is black. */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Streaming")
	bool bBlockOnLevelStreaming = true;

	/** Optional convenience for initial state: if player starts already inside, apply this zone on BeginPlay. */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Trigger")
	bool bApplyIfPlayerStartsInside = false;

	UFUNCTION()
	void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
	bool IsPlayerActor(AActor* Actor) const;
	void EnsureDirector();
	void ApplyZoneToPlayer(AActor* PlayerActor);
	void RestorePlayerCamera();
};
