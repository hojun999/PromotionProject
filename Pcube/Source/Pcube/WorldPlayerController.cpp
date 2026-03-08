#include "WorldPlayerController.h"

#include "WorldCCTVCameraDirector.h"
#include "WorldHUD.h"
#include "EngineUtils.h"

AWorldPlayerController::AWorldPlayerController()
{
	bAutoManageActiveCameraTarget = false;
}

void AWorldPlayerController::BeginPlay()
{
	Super::BeginPlay();

	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);
	bShowMouseCursor = false;

	GetWorldTimerManager().SetTimer(
		TimerHandle_TrySetCCTV,
		this,
		&AWorldPlayerController::TrySetCCTVView,
		0.1f,
		true
	);
}

void AWorldPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(TimerHandle_TrySetCCTV);
	Super::EndPlay(EndPlayReason);
}

void AWorldPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	TrySetCCTVView();
}

void AWorldPlayerController::TrySetCCTVView()
{
	APawn* P = GetPawn();
	if (!P) return;

	if (!CachedDirector)
	{
		for (TActorIterator<AWorldCCTVCameraDirector> It(GetWorld()); It; ++It)
		{
			CachedDirector = *It;
			break;
		}
	}

	if (!CachedDirector) return;

	if (GetViewTarget() == CachedDirector)
	{
		GetWorldTimerManager().ClearTimer(TimerHandle_TrySetCCTV);
		return;
	}

	CachedDirector->SetTargetActor(P);
	SetViewTargetWithBlend(CachedDirector, 0.0f);
	GetWorldTimerManager().ClearTimer(TimerHandle_TrySetCCTV);
}

void AWorldPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	InputComponent->BindKey(EKeys::I, IE_Pressed, this, &AWorldPlayerController::Input_ToggleInventory);
	InputComponent->BindKey(EKeys::Tab, IE_Pressed, this, &AWorldPlayerController::Input_TogglePlayerInfo);
	InputComponent->BindKey(EKeys::Escape, IE_Pressed, this, &AWorldPlayerController::Input_CloseUI);
}

bool AWorldPlayerController::IsCameraTransitionBusy() const
{
	return CachedDirector && CachedDirector->IsZoneTransitionInProgress();
}

void AWorldPlayerController::Input_ToggleInventory()
{
	if (IsCameraTransitionBusy())
	{
		return;
	}

	if (AWorldHUD* WHUD = Cast<AWorldHUD>(GetHUD()))
	{
		WHUD->ToggleInventory();
	}
}

void AWorldPlayerController::Input_TogglePlayerInfo()
{
	if (IsCameraTransitionBusy())
	{
		return;
	}

	if (AWorldHUD* WHUD = Cast<AWorldHUD>(GetHUD()))
	{
		WHUD->TogglePlayerInfo();
	}
}

void AWorldPlayerController::Input_CloseUI()
{
	if (IsCameraTransitionBusy())
	{
		return;
	}

	if (AWorldHUD* WHUD = Cast<AWorldHUD>(GetHUD()))
	{
		WHUD->CloseAnyOpenPanel();
	}
}
