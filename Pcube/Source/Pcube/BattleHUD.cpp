// Fill out your copyright notice in the Description page of Project Settings.


#include "BattleHUD.h"

#include "AllyStatusPanelWidget.h"
#include "BattleAllyUnit.h"
#include "BattleBaseUnit.h"
#include "BattlePlayerController.h"
#include "BattleActionIntentWidget.h"
#include "ItemDataAsset.h"
#include "SkillDataAsset.h"
#include "Blueprint/UserWidget.h"
#include "AudioManagerSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "SettingsWidget.h"

void ABattleHUD::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Warning, TEXT("BattleHUD: BeginPlay Started"));

	if (BattleBGMSound)
	{
		if (UGameInstance* GI = GetGameInstance())
		{
			if (UAudioManagerSubsystem* AM = GI->GetSubsystem<UAudioManagerSubsystem>())
			{
				AM->PlayBGM(BattleBGMSound);
			}
		}
	}

	CreateAllWidgets();
	BindSubsystemEvents();
	
	if (ABattlePlayerController* BPC = Cast<ABattlePlayerController>(GetOwningPlayerController()))
	{
		UE_LOG(LogTemp, Warning, TEXT("[HUD] NotifyBattleHUDReady -> PC"));
		BPC->NotifyBattleHUDReady(this);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[HUD] OwningPlayerController is not ABattlePlayerController"));
	}
}

void ABattleHUD::CreateAllWidgets()
{
	APlayerController* PC = GetOwningPlayerController();
	if (!PC) return;
	
	// 행동 순서 UI
	if (ActionOrderClass)
	{
		ActionOrderWidget = CreateWidget<UBattleActionOrderWidget>(PC, ActionOrderClass);
		if (ActionOrderWidget)
		{
			ActionOrderWidget->AddToViewport(1);
		}
	}
	
	// 행동 선택 UI 
	if (ActionMenuClass)
	{
		BattleActionMenuWidget = CreateWidget<UBattleActionMenu>(PC, ActionMenuClass);
		if (BattleActionMenuWidget)
		{
			BattleActionMenuWidget->AddToViewport(50);
			BattleActionMenuWidget->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
	
	// 아군 유닛 초상화 및 체력 상태 위젯 생성
	if (AllyStatusPanelClass)
	{
		AllyStatusPanelWidget = CreateWidget<UAllyStatusPanelWidget>(PC, AllyStatusPanelClass);
		if (AllyStatusPanelWidget)
		{
			AllyStatusPanelWidget->AddToViewport(10);
			AllyStatusPanelWidget->SetVisibility(ESlateVisibility::Visible);
		}
	}
	
	// 전투 진행에 따른 텍스트 출력 위젯 생성
	if (BattleStateNoticeClass)
	{
		BattleStateNoticeWidget = CreateWidget<UUserWidget>(PC, BattleStateNoticeClass);
		if (BattleStateNoticeWidget)
		{
			BattleStateNoticeWidget->AddToViewport(10);
		}
	}
	
	if (GameOverWidgetClass)
	{
		GameOverWidget = CreateWidget<UUserWidget>(PC, GameOverWidgetClass);
		if (GameOverWidget)
		{
			GameOverWidget->AddToViewport(10);
			GameOverWidget->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
	
	if (VictoryWidgetClass)
	{
		VictoryWidget = CreateWidget<UUserWidget>(PC, VictoryWidgetClass);
		if (VictoryWidget)
		{
			VictoryWidget->AddToViewport(10);
			VictoryWidget->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	// 게임 클리어 위젯 생성 // 추가됨
	if (GameClearWidgetClass)
	{
		GameClearWidget = CreateWidget<UUserWidget>(PC, GameClearWidgetClass);
		if (GameClearWidget)
		{
			GameClearWidget->AddToViewport(200);
			GameClearWidget->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
	
	// Q/E 타겟 변경 힌트 위젯 생성 
	if (ChangeTargetHintWidgetClass) 
	{ 
		DisplayChangeTargetWidget = CreateWidget<UUserWidget>(PC, ChangeTargetHintWidgetClass); 
		if (DisplayChangeTargetWidget) 
		{ 
			DisplayChangeTargetWidget->AddToViewport(20); 
			DisplayChangeTargetWidget->SetVisibility(ESlateVisibility::Collapsed); 
		} 
	} 

	// 우클릭 돌아가기 힌트 위젯 생성 
	if (CancelHintWidgetClass) 
	{ 
		DisplayCancelWidget = CreateWidget<UUserWidget>(PC, CancelHintWidgetClass); 
		if (DisplayCancelWidget) 
		{ 
			DisplayCancelWidget->AddToViewport(20); 
			DisplayCancelWidget->SetVisibility(ESlateVisibility::Collapsed); 
		} 
	} 
	
	if (SettingsWidgetClass)
	{
		
	}
}


void ABattleHUD::BindSubsystemEvents()
{
	if (UWorld* World = GetWorld())
	{
		if (UBattleControlSubsystem* BattleControlSub = World->GetSubsystem<UBattleControlSubsystem>())
		{
			BattleControlSub->OnBattleStateChanged.RemoveDynamic(this, &ABattleHUD::HandleBattleStateChanged);
			BattleControlSub->OnTurnUnitChanged.RemoveDynamic(this, &ABattleHUD::HandleTurnUnitChanged);
			BattleControlSub->OnTurnOrderChanged.RemoveDynamic(this, &ABattleHUD::HandleActionOrderChanged);
			BattleControlSub->OnTargetChanged.RemoveDynamic(this, &ABattleHUD::HandleTargetChanged);
			BattleControlSub->OnBattleUnitsSpawned.RemoveDynamic(this, &ABattleHUD::HandleBattleUnitSpawned);
			BattleControlSub->OnBattleFinished.RemoveDynamic(this, &ABattleHUD::HandleBattleFinished);

			BattleControlSub->OnBattleStateChanged.AddDynamic(this, &ABattleHUD::HandleBattleStateChanged);
			BattleControlSub->OnTurnUnitChanged.AddDynamic(this, &ABattleHUD::HandleTurnUnitChanged);
			BattleControlSub->OnTurnOrderChanged.AddDynamic(this, &ABattleHUD::HandleActionOrderChanged);
			BattleControlSub->OnTargetChanged.AddDynamic(this, &ABattleHUD::HandleTargetChanged);
			BattleControlSub->OnBattleUnitsSpawned.AddDynamic(this, &ABattleHUD::HandleBattleUnitSpawned);
			BattleControlSub->OnBattleFinished.AddDynamic(this, &ABattleHUD::HandleBattleFinished);
		}
	}
}

void ABattleHUD::HandleBattleStateChanged(EBattleState NewState)
{
	if (NewState == EBattleState::ActionInput || NewState == EBattleState::Finished)
	{
		HideActionIntent();
	}

	// 타겟 선택 상태 진입 시 Q/E/F 힌트 표시 (살아있는 적 2개 이상일 때만) 
	// 아군 턴 여부 확인
	const bool bIsAllyTurn = IsValid(CurrentTurnUnit) && CurrentTurnUnit->IsA<ABattleAllyUnit>();

	if (DisplayChangeTargetWidget)
	{
		const bool bShowQE = bIsAllyTurn && (NewState == EBattleState::TargetSelection) && HasMultipleAliveEnemies();
		DisplayChangeTargetWidget->SetVisibility(bShowQE ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}

	if (DisplayCancelWidget)
	{
		const bool bShowCancel = bIsAllyTurn && (NewState == EBattleState::TargetSelection);
		DisplayCancelWidget->SetVisibility(bShowCancel ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}
}



void ABattleHUD::HandleBattleUnitSpawned(const TArray<AActor*>& Allies, const TArray<AActor*>& Enemies)
{
	UE_LOG(LogTemp, Warning, TEXT("[HUD] HandleBattleUnitSpawned Allies=%d Enemies=%d Panel=%s"),
		Allies.Num(), Enemies.Num(), *GetNameSafe(AllyStatusPanelWidget));

	if (AllyStatusPanelWidget)
	{
		AllyStatusPanelWidget->InitParty(Allies);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[HUD] AllyStatusPanelWidget is NULL (cannot InitParty)"));
	}

	for (AActor* EnemyActor : Enemies)
	{
		if (ABattleBaseUnit* EnemyUnit = Cast<ABattleBaseUnit>(EnemyActor))
		{
			EnemyUnit->SetEnemyHPBarVisible(true);
			EnemyUnit->RefreshEnemyHPBar();
		}
	}
}

void ABattleHUD::HandleTurnUnitChanged(ABattleBaseUnit* ActiveUnit)
{
	CurrentTurnUnit = ActiveUnit;
	HideActionIntent();

	if (!BattleActionMenuWidget || !ActiveUnit) return;

	if (ABattleAllyUnit* AllyUnit = Cast<ABattleAllyUnit>(ActiveUnit))
	{
		if (BattleActionMenuWidget)
		{
			BattleActionMenuWidget->ShowMenu(AllyUnit);
		}
	
		if (AllyStatusPanelWidget)
		{
			AllyStatusPanelWidget->SetActiveUnit(AllyUnit);
		}
	}
	else
	{
		// 적 턴이면 ActiveBorder 끄기
		if (AllyStatusPanelWidget)
		{
			AllyStatusPanelWidget->SetActiveUnit(nullptr);
		}
	}
}

void ABattleHUD::ShowActionMenu(ABattleAllyUnit* AllyUnit)
{
	UE_LOG(LogTemp, Warning, TEXT("[HUD] ShowActionMenu unit=%s widget=%s visBefore=%d inViewport=%d"),
	*GetNameSafe(AllyUnit),
	*GetNameSafe(BattleActionMenuWidget),
	BattleActionMenuWidget ? (int32)BattleActionMenuWidget->GetVisibility() : -1,
	BattleActionMenuWidget ? (int32)BattleActionMenuWidget->IsInViewport() : -1);

	if (!BattleActionMenuWidget || !AllyUnit) return;

	BattleActionMenuWidget->ShowMenu(AllyUnit);
	BattleActionMenuWidget->SetVisibility(ESlateVisibility::Visible);

	UE_LOG(LogTemp, Warning, TEXT("[HUD] Menu visAfter=%d inViewport=%d"),
	(int32)BattleActionMenuWidget->GetVisibility(),
	(int32)BattleActionMenuWidget->IsInViewport());
}

void ABattleHUD::HideActionMenu()
{
	if (!BattleActionMenuWidget) return;
	BattleActionMenuWidget->SetVisibility(ESlateVisibility::Collapsed);
}

void ABattleHUD::ShowActionIntentForSkill(USkillDataAsset* Skill)
{
	if (ActionIntentWidget)
	{
		ActionIntentWidget->ShowSkillIntent(Skill);
	}
}

void ABattleHUD::ShowActionIntentForItem(UItemDataAsset* Item)
{
	if (ActionIntentWidget)
	{
		ActionIntentWidget->ShowItemIntent(Item);
	}
}

void ABattleHUD::HideActionIntent()
{
	if (ActionIntentWidget)
	{
		ActionIntentWidget->ClearIntent();
	}
}

void ABattleHUD::ShowGameOverUI()
{
	HideActionIntent();
	if (GameOverWidget) GameOverWidget->SetVisibility(ESlateVisibility::Visible);
}

void ABattleHUD::ShowVictoryUI()
{
	HideActionIntent();
	if (VictoryWidget) VictoryWidget->SetVisibility(ESlateVisibility::Visible);
}

void ABattleHUD::ShowGameClearUI()
{
	HideActionIntent();
	if (GameClearWidget)
	{
		GameClearWidget->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		// GameClearWidgetClass 미할당 시 로그 출력
		UE_LOG(LogTemp, Warning, TEXT("[BattleHUD] GameClearWidget is null - assign GameClearWidgetClass in BP"));
	}
}

void ABattleHUD::HandleActionOrderChanged(const TArray<AActor*>& NewOrder)
{
	UE_LOG(LogTemp, Warning, TEXT("BattleHUD: HandleActionOrderChanged Called! Unit Count: %d"), NewOrder.Num());
	
	if (ActionOrderWidget)
	{
		ActionOrderWidget->RefreshList(NewOrder);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("BattleHUD: TurnOrderWidget is NULL!"));
	}
}

void ABattleHUD::HandleTargetChanged(AActor* NewTarget)
{
	if (DisplayChangeTargetWidget)
	{
		// 아군 턴 + 타겟 있음 + 살아있는 적 2개 이상일 때만 표시
		const bool bIsAllyTurn = IsValid(CurrentTurnUnit) && CurrentTurnUnit->IsA<ABattleAllyUnit>();
		const bool bShowQE = bIsAllyTurn && NewTarget && HasMultipleAliveEnemies();
		DisplayChangeTargetWidget->SetVisibility(bShowQE ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}
}

bool ABattleHUD::HasMultipleAliveEnemies() const 
{ 
	UWorld* World = GetWorld(); 
	if (!World) return false; 

	UBattleControlSubsystem* Sub = World->GetSubsystem<UBattleControlSubsystem>(); 
	if (!Sub) return false; 

	return Sub->GetAliveUnitCount(Sub->SpawnedEnemies) >= 2; 
} 

void ABattleHUD::HandleBattleFinished(EBattleResult Result)
{
	HideActionMenu();

	// 전투 종료 시 BGM 정지 - 월드 복귀 후 WorldHUD가 PlayBGM 호출
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UAudioManagerSubsystem* AM = GI->GetSubsystem<UAudioManagerSubsystem>())
		{
			AM->StopBGM(1.0f);
		}
	}

	if (Result == EBattleResult::Victory)
	{
		ShowVictoryUI();
	}
	else if (Result == EBattleResult::Defeat)
	{
		ShowGameOverUI();
	}
}

void ABattleHUD::OnSettingsOpened()
{
	UGameplayStatics::SetGamePaused(this, true);
	//HideActionMenu();

	// 설정창 오픈 시 마우스 커서 표시 및 포커스 강제 설정
	if (ABattlePlayerController* BPC = Cast<ABattlePlayerController>(GetOwningPlayerController()))
	{
		FInputModeGameAndUI Mode;
		// BaseHUD에 있는 SettingsWidget을 포커스 (변수명은 프로젝트에 맞춰 수정)
		if (SettingsWidget) 
		{
			Mode.SetWidgetToFocus(SettingsWidget->TakeWidget());
		}
		BPC->SetInputMode(Mode);
		BPC->bShowMouseCursor = true;
	}
}

void ABattleHUD::OnSettingsClosed()
{
	UGameplayStatics::SetGamePaused(this, false);

	// 현재 전투 상태를 확인하여 UI 및 입력 모드 복구
	if (UWorld* World = GetWorld())
	{
		if (UBattleControlSubsystem* BattleSub = World->GetSubsystem<UBattleControlSubsystem>())
		{
			ABattlePlayerController* BPC = Cast<ABattlePlayerController>(GetOwningPlayerController());
			if (!BPC) return;

			if (BattleSub->GetCurrentState() == EBattleState::ActionInput)
			{
				// 아군 턴인 경우 액션 메뉴 다시 표시
				if (ABattleAllyUnit* AllyUnit = Cast<ABattleAllyUnit>(CurrentTurnUnit))
				{
					ShowActionMenu(AllyUnit);
					BPC->bShowMouseCursor = true;
					if (BattleActionMenuWidget)
					{
						FInputModeGameAndUI Mode;
						Mode.SetWidgetToFocus(BattleActionMenuWidget->TakeWidget());
						BPC->SetInputMode(Mode);
					}
				}
			}
			else if (BattleSub->GetCurrentState() == EBattleState::TargetSelection)
			{
				// 타겟 선택 중이었다면 게임 모드로 복귀
				FInputModeGameOnly Mode;
				BPC->SetInputMode(Mode);
				BPC->bShowMouseCursor = true;
			}
			else
			{
				// 그 외(연출 중 등)에는 커서 숨김
				FInputModeGameOnly Mode;
				BPC->SetInputMode(Mode);
				BPC->bShowMouseCursor = false;
			}
		}
	}
}

