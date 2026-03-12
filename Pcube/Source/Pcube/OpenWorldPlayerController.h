#pragma once

#include "CoreMinimal.h"
#include "WorldPlayerController.h"
#include "OpenWorldPlayerController.generated.h"

/**
 * L_OpenWorld 전용 PlayerController.
 * WorldPlayerController를 상속하여 UI(WorldHUD) 입력 처리를 그대로 재사용하고
 * CCTV 카메라 의존 부분(TrySetCCTVView, IsCameraTransitionBusy)만 오버라이드한다.
 *
 * 카메라: 별도 CCTVCameraDirector 없이 폰 자체를 뷰타겟으로 사용.
 * 폰(WorldAllyUnit)에 SpringArm + CineCameraComponent가 붙어있으면 자동으로 그 카메라를 사용.
 * 없으면 폰의 기본 시점(3인칭 위치)으로 폴백.
 */
UCLASS()
class PCUBE_API AOpenWorldPlayerController : public AWorldPlayerController
{
	GENERATED_BODY()

public:
	AOpenWorldPlayerController();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

protected:
	virtual void OnPossess(APawn* InPawn) override;

	/** CCTV 전환 중 여부 - 오픈월드는 항상 false */
	virtual bool IsCameraTransitionBusy() const override; // 추가됨

private:
	/** 폰을 뷰타겟으로 설정. 폰이 준비되기 전이면 타이머로 재시도 */
	void TrySetPawnView(); // 추가됨

	FTimerHandle TimerHandle_TrySetPawnView; // 추가됨
};
