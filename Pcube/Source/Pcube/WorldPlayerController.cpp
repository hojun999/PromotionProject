// Fill out your copyright notice in the Description page of Project Settings.


#include "WorldPlayerController.h"

#include "WorldCCTVCameraDirector.h"
#include "WorldHUD.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"


AWorldPlayerController::AWorldPlayerController()
{
	bAutoManageActiveCameraTarget = false;
}

void AWorldPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	// 이동 레벨에서는 마우스 커서 숨기기
	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);
	bShowMouseCursor = false;
	
	// Pawn 아직 없거나 Director 아직 없을 수 있으니 재시도 타이머
	GetWorldTimerManager().SetTimer(
		TimerHandle_TrySetCCTV,
		this,
		&AWorldPlayerController::TrySetCCTVView,
		0.1f,
		true
	);
}

void AWorldPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	
	// Possess 후에도 한 번 즉시 시도
	TrySetCCTVView();
}

void AWorldPlayerController::TrySetCCTVView()
{
	APawn* P = GetPawn();
	if (!P) return;

	if (!CachedDirector)
	{
		// 가장 확실한 방식: TActorIterator
		for (TActorIterator<AWorldCCTVCameraDirector> It(GetWorld()); It; ++It)
		{
			CachedDirector = *It;
			break;
		}
	}

	if (!CachedDirector) return;

	// 이미 ViewTarget이 Director면 끝
	if (GetViewTarget() == CachedDirector)
	{
		GetWorldTimerManager().ClearTimer(TimerHandle_TrySetCCTV);
		return;
	}

	CachedDirector->SetTargetActor(P);
	SetViewTargetWithBlend(CachedDirector, 0.0f);

	// 성공했으면 타이머 종료
	GetWorldTimerManager().ClearTimer(TimerHandle_TrySetCCTV);
}

void AWorldPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	
	InputComponent->BindKey(EKeys::I, IE_Pressed, this, &AWorldPlayerController::Input_ToggleInventory);
	InputComponent->BindKey(EKeys::Tab, IE_Pressed, this, &AWorldPlayerController::Input_TogglePlayerInfo);
	InputComponent->BindKey(EKeys::Escape, IE_Pressed, this, &AWorldPlayerController::Input_CloseUI);
}

void AWorldPlayerController::Input_ToggleInventory()
{
	if (AWorldHUD* WHUD = Cast<AWorldHUD>(GetHUD()))
	{
		WHUD->ToggleInventory();
	}
}

void AWorldPlayerController::Input_TogglePlayerInfo()
{
	if (AWorldHUD* WHUD = Cast<AWorldHUD>(GetHUD()))
	{
		WHUD->TogglePlayerInfo();
	}
}

void AWorldPlayerController::Input_CloseUI()
{
	if (AWorldHUD* WHUD = Cast<AWorldHUD>(GetHUD()))
	{
		WHUD->CloseAnyOpenPanel();
	}
}