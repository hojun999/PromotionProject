#include "OpenWorldPlayerController.h"

#include "WorldHUD.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

AOpenWorldPlayerController::AOpenWorldPlayerController()
{
	// 오픈월드는 폰 자체가 카메라를 가지므로 자동 뷰타겟 관리를 엔진에 맡김
	bAutoManageActiveCameraTarget = true; 
}

void AOpenWorldPlayerController::BeginPlay()
{
	// 부모의 BeginPlay는 CCTV 타이머를 세팅하므로 호출하지 않고 직접 처리
	// Super::BeginPlay() 의도적으로 미호출 - CCTV 타이머 방지 
	APlayerController::BeginPlay(); // APlayerController 레벨까지만 호출 

	FInputModeGameOnly InputMode; 
	SetInputMode(InputMode); 
	bShowMouseCursor = false; 

	// 폰이 아직 준비 안 됐을 수 있으므로 타이머로 재시도 
	GetWorldTimerManager().SetTimer( 
		TimerHandle_TrySetPawnView, 
		this, 
		&AOpenWorldPlayerController::TrySetPawnView, 
		0.1f, 
		true 
	); 

}

void AOpenWorldPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(TimerHandle_TrySetPawnView); 
	APlayerController::EndPlay(EndPlayReason); 
}

void AOpenWorldPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	TrySetPawnView(); 
}

void AOpenWorldPlayerController::TrySetPawnView()
{
	APawn* P = GetPawn(); 
	if (!P) return; 

	// 이미 폰이 뷰타겟이면 타이머 종료 
	if (GetViewTarget() == P) 
	{ 
		GetWorldTimerManager().ClearTimer(TimerHandle_TrySetPawnView); 
		return; 
	} 

	// 폰을 뷰타겟으로 설정. 블렌드 없이 즉시 전환 
	SetViewTargetWithBlend(P, 0.0f); 
	GetWorldTimerManager().ClearTimer(TimerHandle_TrySetPawnView); 
}

bool AOpenWorldPlayerController::IsCameraTransitionBusy() const
{
	// 오픈월드는 CCTV 전환이 없으므로 항상 false 
	return false; 
}
