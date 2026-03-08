// Fill out your copyright notice in the Description page of Project Settings.


#include "BattleHUD.h"

#include "AllyStatusPanelWidget.h"
#include "BattleAllyUnit.h"
#include "BattleBaseUnit.h"
#include "BattlePlayerController.h"
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
	
	if (ActionOrderClass)
	{
		ActionOrderWidget = CreateWidget<UBattleActionOrderWidget>(PC, ActionOrderClass);
		if (ActionOrderWidget)
		{
			ActionOrderWidget->AddToViewport(1);
		}
	}
	
	if (ActionMenuClass)
	{
		BattleActionMenuWidget = CreateWidget<UBattleActionMenu>(PC, ActionMenuClass);
		if (BattleActionMenuWidget)
		{
			BattleActionMenuWidget->AddToViewport(50);
			BattleActionMenuWidget->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
	
	if (AllyStatusPanelClass)
	{
		AllyStatusPanelWidget = CreateWidget<UAllyStatusPanelWidget>(PC, AllyStatusPanelClass);
		if (AllyStatusPanelWidget)
		{
			AllyStatusPanelWidget->AddToViewport(10);
			AllyStatusPanelWidget->SetVisibility(ESlateVisibility::Visible);
		}
	}
	
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
	// 전투 상태 텍스트/연출 위젯은 이후 BattleStateNoticeWidget 쪽에서 확장
}

void ABattleHUD::HandleBattleUnitSpawned(const TArray<AActor*>& Allies, const TArray<AActor*>& Enemies)
{
	if (AllyStatusPanelWidget)
	{
		AllyStatusPanelWidget->InitParty(Allies);
	}
}

void ABattleHUD::HandleTurnUnitChanged(ABattleBaseUnit* ActiveUnit)
{
	if (!ActiveUnit)
	{
		HideActionMenu();
		if (AllyStatusPanelWidget)
		{
			AllyStatusPanelWidget->SetActiveUnit(nullptr);
		}
		return;
	}

	if (ABattleAllyUnit* AllyUnit = Cast<ABattleAllyUnit>(ActiveUnit))
	{
		ShowActionMenu(AllyUnit);
		if (AllyStatusPanelWidget)
		{
			AllyStatusPanelWidget->SetActiveUnit(AllyUnit);
		}
	}
	else
	{
		HideActionMenu();
		if (AllyStatusPanelWidget)
		{
			AllyStatusPanelWidget->SetActiveUnit(nullptr);
		}
	}
}

void ABattleHUD::ShowActionMenu(ABattleAllyUnit* AllyUnit)
{
	if (!BattleActionMenuWidget || !AllyUnit) return;
	
	BattleActionMenuWidget->ShowMenu(AllyUnit);
	BattleActionMenuWidget->SetVisibility(ESlateVisibility::Visible);
}

void ABattleHUD::HideActionMenu()
{
	if (!BattleActionMenuWidget) return;
	BattleActionMenuWidget->SetVisibility(ESlateVisibility::Collapsed);
}

void ABattleHUD::ShowGameOverUI()
{
	if (GameOverWidget) GameOverWidget->SetVisibility(ESlateVisibility::Visible);
}

void ABattleHUD::ShowVictoryUI()
{
	if (VictoryWidget) VictoryWidget->SetVisibility(ESlateVisibility::Visible);
}

void ABattleHUD::HandleActionOrderChanged(const TArray<AActor*>& NewOrder)
{
	if (ActionOrderWidget)
	{
		ActionOrderWidget->RefreshList(NewOrder);
	}
}

void ABattleHUD::HandleTargetChanged(AActor* NewTarget)
{
	// 타겟 강조/하이라이트 연동 지점
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
