#include "WorldPlayerController.h"

#include "WorldCCTVCameraDirector.h"
#include "WorldHUD.h"
#include "BaseHUD.h"
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
	
	// bExecuteWhenPaused를 true로 설정하여 일시정지 중에도 ESC가 작동하게 함
	FInputKeyBinding& EscBinding = InputComponent->BindKey(EKeys::Escape, IE_Pressed, this, &AWorldPlayerController::Input_CloseUI); // 수정됨
	EscBinding.bExecuteWhenPaused = true; // 추가됨
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
	if (IsCameraTransitionBusy()) return;

	ABaseHUD* BaseHUD = Cast<ABaseHUD>(GetHUD());
	AWorldHUD* WHUD = Cast<AWorldHUD>(BaseHUD);
	if (!BaseHUD) return;

	// 1. 설정창이 열려있으면 최우선으로 닫기
	if (BaseHUD->IsSettingsOpen()) 
	{
		BaseHUD->ToggleSettings(); 
		return; 
	}

	// 2. 다른 UI(인벤토리 등)가 열려있으면 그것을 닫기
	if (WHUD && WHUD->CloseAnyOpenPanel())
	{
		return; // 패널을 닫았다면 로직 종료
	}

	// 3. 아무것도 열려있지 않다면 설정창 열기
	BaseHUD->ToggleSettings();
}
