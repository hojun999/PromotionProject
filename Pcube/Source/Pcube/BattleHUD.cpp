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

void ABattleHUD::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Warning, TEXT("BattleHUD: BeginPlay Started"));
	
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
		DisplayCancleWidget = CreateWidget<UUserWidget>(PC, CancelHintWidgetClass); 
		if (DisplayCancleWidget) 
		{ 
			DisplayCancleWidget->AddToViewport(20); 
			DisplayCancleWidget->SetVisibility(ESlateVisibility::Collapsed); 
		} 
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

	// 타겟 선택 상태 진입 시 Q/E 힌트 표시 (살아있는 적 2개 이상일 때만) 
	if (DisplayChangeTargetWidget) 
	{ 
		const bool bShowQE = (NewState == EBattleState::TargetSelection) && HasMultipleAliveEnemies(); 
		DisplayChangeTargetWidget->SetVisibility(bShowQE ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed); 
	} 

	// 타겟 선택 / 스킬 리스트 / 아이템 상태에서 우클릭 돌아가기 힌트 표시 
	if (DisplayCancleWidget) 
	{ 
		// 타겟 선택 상태에서만 표시 - ActionInput(액션메뉴 표시 중)일 때는 숨김 
		const bool bShowCancel = (NewState == EBattleState::TargetSelection); 
		DisplayCancleWidget->SetVisibility(bShowCancel ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed); 
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

		// 적 턴이면 메뉴 숨기기
		// if (BattleActionMenuWidget)
		// {
		// 	BattleActionMenuWidget->SetVisibility(ESlateVisibility::Collapsed);
		// }
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
		// 타겟이 있고 살아있는 적이 2개 이상일 때만 QE 힌트 표시 
		const bool bShowQE = NewTarget && HasMultipleAliveEnemies(); 
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

	if (Result == EBattleResult::Victory)
	{
		ShowVictoryUI();
	}
	else if (Result == EBattleResult::Defeat)
	{
		ShowGameOverUI();
	}
}
