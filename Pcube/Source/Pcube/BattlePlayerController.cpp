// Fill out your copyright notice in the Description page of Project Settings.


#include "BattlePlayerController.h"
#include "BattleControlSubsystem.h"
#include "BattleHUD.h"
#include "BattleBaseUnit.h"
#include "BattleAllyUnit.h"
#include "BattleInfoTransferSubsystem.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

void ABattlePlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	// 로컬 플레이어만 카메라/입력 제어
	if (!IsLocalController())
	{
		return;
	}
	
	BattleSub = GetWorld() ? GetWorld()->GetSubsystem<UBattleControlSubsystem>() : nullptr;
	CacheBattleHUD();
	BindBattleDelegates();
	
	// 기본 입력 상태 설정
	//ApplyInputMode_GameOnly(false);
	
	// bShowMouseCursor = false;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
	DefaultMouseCursor = EMouseCursor::Default;
}

void ABattlePlayerController::CacheBattleHUD()
{
	BattleHUD = Cast<ABattleHUD>(GetHUD());
}

void ABattlePlayerController::BindBattleDelegates()
{
	if (!BattleSub)
	{
		UE_LOG(LogTemp, Error, TEXT("BattlePlayerController: BattleControlSubsystem is null"));
		return;
	}
	
	BattleSub->OnTurnUnitChanged.AddDynamic(this, &ABattlePlayerController::HandleTurnUnitChanged);
	BattleSub->OnTargetChanged.AddDynamic(this, &ABattlePlayerController::HandleTargetChanged);
	BattleSub->OnBattleStateChanged.AddDynamic(this, &ABattlePlayerController::HandleBattleStateChanged);
	BattleSub->OnBattleFinished.AddDynamic(this, &ABattlePlayerController::HandleBattleFinished);
}

void ABattlePlayerController::UnbindBattleDelegates()
{
	BattleSub->OnTurnUnitChanged.RemoveDynamic(this, &ABattlePlayerController::HandleTurnUnitChanged);
	BattleSub->OnTargetChanged.RemoveDynamic(this, &ABattlePlayerController::HandleTargetChanged);
	BattleSub->OnBattleStateChanged.RemoveDynamic(this, &ABattlePlayerController::HandleBattleStateChanged);
	BattleSub->OnBattleFinished.RemoveDynamic(this, &ABattlePlayerController::HandleBattleFinished);
}

void ABattlePlayerController::NotifyBattleHUDReady(ABattleHUD* HUD)
{
	BattleHUD = HUD;
	BindWidgetRequests();
}

void ABattlePlayerController::BindWidgetRequests()
{
	if (!BattleHUD)
	{
		UE_LOG(LogTemp, Error, TEXT("[PC] BattleHUD null"));
		return;
	}
	
	UBattleActionMenu* ActionMenu = BattleHUD->GetActionMenuWidget();
	
	UE_LOG(LogTemp, Warning, TEXT("[PC] BindWidgetRequests Menu=%s"), *GetNameSafe(ActionMenu));
	if (!ActionMenu) return;
	
	// 중복 바인딩 방지
	if (!ActionMenu->OnActionRequested.IsAlreadyBound(this, &ABattlePlayerController::HandleActionRequested))
	{
		ActionMenu->OnActionRequested.AddDynamic(this, &ABattlePlayerController::HandleActionRequested);
	}
	if (!ActionMenu->OnSkillRequested.IsAlreadyBound(this, &ABattlePlayerController::HandleSkillRequested))
	{
		ActionMenu->OnSkillRequested.AddDynamic(this, &ABattlePlayerController::HandleSkillRequested);
	}
	if (!ActionMenu->OnSkillMenuRequested.IsAlreadyBound(this, &ABattlePlayerController::HandleSkillMenuRequested))
	{
		ActionMenu->OnSkillMenuRequested.AddDynamic(this, &ABattlePlayerController::HandleSkillMenuRequested);
	}
	if (!ActionMenu->OnItemMenuRequested.IsAlreadyBound(this, &ABattlePlayerController::HandleItemMenuRequested))
	{
		ActionMenu->OnItemMenuRequested.AddDynamic(this, &ABattlePlayerController::HandleItemMenuRequested);
	}
	
}

void ABattlePlayerController::HandleActionRequested(USkillDataAsset* SkillData)
{
	if (!BattleSub) BattleSub = GetWorld() ? GetWorld()->GetSubsystem<UBattleControlSubsystem>() : nullptr;
	if (!BattleSub || !SkillData) return;
	
	// 1. 메뉴 숨기기
	if (BattleHUD) BattleHUD->HideActionMenu();
	
	// 2. 타겟 선택 전환
	// 마우스로 적 클릭하여 선택
	ApplyInputMode_GameOnly(true);
	
	// 3. 전투 호출
	BattleSub->StartTargetSelection(SkillData);
}

void ABattlePlayerController::HandleSkillRequested(USkillDataAsset* Skill)
{
	if (!IsValid(Skill)) return;
	
	// 전체 타겟 -> default camera로 전환
	if (Skill->TargetType == ESkillTargetRule::AllEnemis ||
		Skill->TargetType == ESkillTargetRule::AllAllies)
	{
		// 방법 1: PC에 함수 구현
		FocusDefaultBattleCamera(0.3f);
		
		// 방법 2: BattlecontrolSubsystem에 OnTargetChanged(nullptr) 전달 후
		// PC가 nullptr이면 default로 처리하는 로직
	}
	
	if (UBattleControlSubsystem* Sub = GetWorld()->GetSubsystem<UBattleControlSubsystem>())
	{
		Sub->RequestUseSkill(Skill);
	}
}

void ABattlePlayerController::HandleSkillMenuRequested()
{
	// TODO: 스킬 리스트 UI를 HUD를 통해 보여주도록 호출
	// 입력은 GameAndUI 유지
	if (BattleHUD)
	{
		if (UUserWidget* ActionMenu = BattleHUD->GetActionMenuWidget())
		{
			ApplyInputMode_GameAndUI(ActionMenu, true);
		}
	}
}

void ABattlePlayerController::HandleItemMenuRequested()
{
	// TODO: 아이템 리스트 UI를 HUD를 통해 보여주도록 호출
	// 입력은 GameAndUI 유지
	if (BattleHUD)
	{
		if (UUserWidget* ActionMenu = BattleHUD->GetActionMenuWidget())
		{
			ApplyInputMode_GameAndUI(ActionMenu, true);
		}
	}
}

void ABattlePlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnbindBattleDelegates();
	Super::EndPlay(EndPlayReason);
}

void ABattlePlayerController::FocusViewTarget(AActor* Target, float BlendTime)
{
	if (!IsValid(Target)) return;
	
	// 시네마틱 카메라가 붙어있는 유닛 Actor로 뷰타겟 전환
	SetViewTargetWithBlend(Target, BlendTime, VTBlend_Cubic);
}

void ABattlePlayerController::FocusDefaultBattleCamera(float BlendTime)
{
	// TODO: default camera로 전환
}

void ABattlePlayerController::ApplyInputMode_GameOnly(bool bShowCursor)
{
	FInputModeGameOnly Mode;
	SetInputMode(Mode);
	bShowMouseCursor = bShowCursor;
}

void ABattlePlayerController::ApplyInputMode_GameAndUI(UUserWidget* FocusWidget, bool bShowCursor)
{
	FInputModeGameAndUI Mode;
	if (FocusWidget)
	{
		Mode.SetWidgetToFocus(FocusWidget->TakeWidget());
	}
	Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	
	SetInputMode(Mode);
	bShowMouseCursor = bShowCursor;
}

void ABattlePlayerController::HandleTurnUnitChanged(ABattleBaseUnit* ActiveUnit)
{
	if (!IsValid(ActiveUnit)) return;
	
	// 필요 시점에 HUD null 방지를 위한 재시도
	if (!BattleHUD) CacheBattleHUD();
	
	// 아군 턴
	if (ABattleAllyUnit* AllyUnit = Cast<ABattleAllyUnit>(ActiveUnit))
	{
		FocusViewTarget(AllyUnit, 0.8f);
		
		if (BattleHUD)
		{
			// TODO: 아래 함수 구현
			BattleHUD->ShowActionMenu(AllyUnit);
			ApplyInputMode_GameAndUI(BattleHUD->GetActionMenuWidget(), true);
		}
		else
		{
			// HUD가 없는 경우 입력만 열어둠
			ApplyInputMode_GameOnly(true);
		}
		return;
	}
	
	// 적 턴
	FocusViewTarget(ActiveUnit, 0.4f);
	
	if (BattleHUD)
	{
		// TODO: 아래 함수 구현
		BattleHUD->HideActionMenu();
	}
	
	UE_LOG(LogTemp, Warning, TEXT("[PC] TurnUnit=%s HUD=%s Menu=%s"),
	*GetNameSafe(ActiveUnit),
	*GetNameSafe(BattleHUD),
	BattleHUD ? *GetNameSafe(BattleHUD->GetActionMenuWidget()) : TEXT("null"));
	
	ApplyInputMode_GameOnly(false);
}

void ABattlePlayerController::HandleTargetChanged(AActor* NewTarget)
{
	if (!IsValid(NewTarget)) return;
	
	FocusViewTarget(NewTarget, 0.4f);
	
	// HUD의 타겟 UI/아웃라인 요청은 여기서 호출해도 됨
	// if (BattleHUD) BattleHUD->NotifyTargetChanged(NewTarget);
	
}

void ABattlePlayerController::HandleBattleStateChanged(EBattleState NewState)
{
	// 예시
	// - TargetSelection 상태 -> GameOnly로 두고 키로 이동 및 확정
	// - ActionExecute에서는 입력 잠금
	switch (NewState)
	{
	case EBattleState::TargetSelection:
		ApplyInputMode_GameOnly(true);
		break;
		
	case EBattleState::ActionExecute:
		ApplyInputMode_GameOnly(false);
		break;
		
	default:
		break;
	}
}

void ABattlePlayerController::HandleBattleFinished(EBattleResult Result)
{
	if (!BattleHUD) CacheBattleHUD();
	
	ApplyInputMode_GameAndUI(nullptr, true); // 커서 켜서 UI 조작 가능 - 필요 시 위젯 포커스 지정하기
	
	UGameInstance* GI = GetGameInstance();
	UBattleInfoTransferSubsystem* Transfer = GI ? GI->GetSubsystem<UBattleInfoTransferSubsystem>() : nullptr;
	
	if (Result == EBattleResult::Defeat)
	{
		if (Transfer)
		{
			Transfer->ClearPendingEncounter();
			Transfer->ClearReturnPoint();
		}
		
		UE_LOG(LogTemp, Error, TEXT("[Battle] Defeat -> GameOver -> Open MainMenu Level"));
		if (BattleHUD) BattleHUD->ShowGameOverUI();
		
		// TODO: 메인메뉴 OpenLevel
		
		return;
	}
	
	if (Result == EBattleResult::Victory)
	{
		if (Transfer)
		{
			Transfer->MarkLastEncounterDefeated();
			const FName ReturnWorld = Transfer->GetReturnWorldLevelName();
			Transfer->ClearPendingEncounter(); // 재사용/중복 기록 방지 (권장)
			
			FTimerHandle TH;
			GetWorld()->GetTimerManager().SetTimer(TH, [this, ReturnWorld]()
			{
				UGameplayStatics::OpenLevel(this, ReturnWorld);
			}, 2.0f, false);
			
			return;
		}
	}
		
	// Transfer가 없는 경우 임시 고정 월드 맵
	FTimerHandle TH;
	GetWorld()->GetTimerManager().SetTimer(TH, [this]()
	{
		UGameplayStatics::OpenLevel(this, FName("WorldLevel"));
	}, 2.0f, false);
}

void ABattlePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	
	// InputComponent->BindAction("NextTarget", IE_Pressed, this, &ABattlePlayerController::Input_NextTarget);
	// InputComponent->BindAction("PrevTarget", IE_Pressed, this, &ABattlePlayerController::Input_PrevTarget);
	// InputComponent->BindAction("ConfirmTarget", IE_Pressed, this, &ABattlePlayerController::Input_ConfirmTarget);
	// InputComponent->BindAction("CancelTarget", IE_Pressed, this, &ABattlePlayerController::Input_CancelTarget);
	
	InputComponent->BindKey(EKeys::Q, IE_Pressed, this, &ABattlePlayerController::Input_PrevTarget);
	InputComponent->BindKey(EKeys::E, IE_Pressed, this, &ABattlePlayerController::Input_NextTarget);
	InputComponent->BindKey(EKeys::F, IE_Pressed, this, &ABattlePlayerController::Input_ConfirmTarget);
	InputComponent->BindKey(EKeys::LeftMouseButton, IE_Pressed, this, &ABattlePlayerController::Input_ClickConfirmTarget);
	InputComponent->BindKey(EKeys::Escape, IE_Pressed, this, &ABattlePlayerController::Input_CancelTarget);
}

void ABattlePlayerController::Input_NextTarget()
{
	if (!BattleSub) return;
	if (BattleSub->GetCurrentState() != EBattleState::TargetSelection) return;
	
	if (BattleSub) BattleSub->MoveOnSelection(+1);
}

void ABattlePlayerController::Input_PrevTarget()
{
	if (!BattleSub) return;
	if (BattleSub->GetCurrentState() != EBattleState::TargetSelection) return;
	
	if (BattleSub) BattleSub->MoveOnSelection(-1);
}

void ABattlePlayerController::Input_ConfirmTarget()
{
	if (!BattleSub) return;
	if (BattleSub->GetCurrentState() != EBattleState::TargetSelection) return;
	
	if (BattleSub) BattleSub->ConfirmTarget();
}

void ABattlePlayerController::Input_ClickConfirmTarget()
{
	if (!BattleSub) return;
	if (BattleSub->GetCurrentState() != EBattleState::TargetSelection) return;
	
	FHitResult Hit;
	const bool bHit = GetHitResultUnderCursor(ECC_Visibility, false, Hit);
	if (!bHit) return;
	
	AActor* HitActor = Hit.GetActor();
	if (!IsValid(HitActor)) return;
	
	// 유닛인지 확인 (적/아군 구분은 battlecontrolsubsystem에서 availabletargets로 검증)
	if (!HitActor->IsA<ABattleBaseUnit>()) return;
	
	// 1. 클릭한 적 선택
	BattleSub->SelectTarget(HitActor);
	
	// 2. 즉시 확정
	BattleSub->ConfirmTarget();
}

void ABattlePlayerController::Input_CancelTarget()
{
	if (!BattleSub) return;
	if (BattleSub->GetCurrentState() != EBattleState::TargetSelection) return;
	
	if (BattleSub) BattleSub->CancelTargetSelection();
}