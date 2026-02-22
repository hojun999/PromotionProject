// Fill out your copyright notice in the Description page of Project Settings.


#include "BattleHUD.h"

#include "AllyStatusPanelWidget.h"
#include "BattleAllyUnit.h"
#include "BattleBaseUnit.h"
#include "BattlePlayerController.h"
#include "Blueprint/UserWidget.h"

// 유닛 스폰이 끝난 후, "Battle Start" 텍스트 출력
// 유닛 스폰이 끝난 후, 아군 초상화 및 스탯 UI
// BattleControlSubSystem에서 유닛 스폰이 끝난

// 각 턴이 시작될 때, 갱신된 유닛 행동 순서 UI

// 특정 유닛 턴 때
// 아군인 경우, 일반 공격, 스킬 공격, 아이템 사용의 3가지 버튼 UI
// 적인 경우, X

// 적이 전멸한 경우, "Battle End" 텍스트 출력

// 아군이 전멸한 경우, "You Die" 텍스트 출력

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
		ActionOrderWidget->AddToViewport(1);
		
		UE_LOG(LogTemp, Warning, TEXT("BattleHUD: ActionOrderClass Created"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("BattleHUD: ActionOrderClass is MISSING!"));
	}
	
	// 행동 선택 UI - 무조건 화면에서 보여야 됨 ***
	if (ActionMenuClass)
	{
		BattleActionMenuWidget = CreateWidget<UBattleActionMenu>(PC, ActionMenuClass);
		BattleActionMenuWidget->AddToViewport(50);
		BattleActionMenuWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
	
	// 아군 유닛 초상화 및 체력 상태 위젯 생성
	if (AllyStatusPanelClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[HUD] AllyStatusPanelClass=%s"), *GetNameSafe(AllyStatusPanelClass.Get()));
		
		AllyStatusPanelWidget = CreateWidget<UAllyStatusPanelWidget>(PC, AllyStatusPanelClass);
		if (!AllyStatusPanelWidget)
		{
			UE_LOG(LogTemp, Warning, TEXT("[HUD] AllyStatusPanelClass=%s"), *GetNameSafe(AllyStatusPanelClass.Get()));
		}
		else
		{
			AllyStatusPanelWidget->AddToViewport(10);
			AllyStatusPanelWidget->SetVisibility(ESlateVisibility::Visible);
			//AllyStatusPanelWidget->SetRenderOpacity(1.f);
			
			// 1) 일단 화면 좌상단에 박아 넣기 (안 보이면 “크기 0”/“위젯 내부 문제” 확정)
			// AllyStatusPanelWidget->SetPositionInViewport(FVector2D(50.f, 50.f), false);
			// AllyStatusPanelWidget->SetDesiredSizeInViewport(FVector2D(800.f, 200.f));
			// AllyStatusPanelWidget->SetAlignmentInViewport(FVector2D(0.f, 0.f));
			
			UE_LOG(LogTemp, Warning, TEXT("[HUD] AllyStatusPanelWidget created=%s inViewport=%d vis=%d"),
			*GetNameSafe(AllyStatusPanelWidget),
			AllyStatusPanelWidget->IsInViewport(),
			(int32)AllyStatusPanelWidget->GetVisibility());
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[HUD] AllyStatusPanelClass is MISSING (set it in BP_BattleHUD defaults)"));
	}
	
	// 전투 진행에 따른 텍스트 출력 위젯 생성
	if (BattleStateNoticeClass)
	{
		BattleStateNoticeWidget = CreateWidget<UUserWidget>(PC, BattleStateNoticeClass);
		BattleStateNoticeWidget->AddToViewport(10);
	}
	
	if (GameOverWidgetClass)
	{
		GameOverWidget = CreateWidget<UUserWidget>(PC, GameOverWidgetClass);
		GameOverWidget->AddToViewport(10);
		GameOverWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
	
	if (VictoryWidgetClass)
	{
		VictoryWidget = CreateWidget<UUserWidget>(PC, VictoryWidgetClass);
		VictoryWidget->AddToViewport(10);
		VictoryWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
}


void ABattleHUD::BindSubsystemEvents()
{
	if (UWorld* World = GetWorld())
	{
		if (UBattleControlSubsystem* BattleContorlSub = World->GetSubsystem<UBattleControlSubsystem>())
		{
			BattleContorlSub->OnBattleStateChanged.AddDynamic(this, &ABattleHUD::HandleBattleStateChanged);
			BattleContorlSub->OnTurnUnitChanged.AddDynamic(this, &ABattleHUD::HandleTurnUnitChanged);
			BattleContorlSub->OnTurnOrderChanged.AddDynamic(this, &ABattleHUD::HandleActionOrderChanged);
			BattleContorlSub->OnTargetChanged.AddDynamic(this, &ABattleHUD::HandleTargetChanged);
			BattleContorlSub->OnBattleUnitsSpawned.AddDynamic(this, &ABattleHUD::HandleBattleUnitSpawned);
			
			UE_LOG(LogTemp, Warning, TEXT("BattleHUD: OnTurnOrderChanged Bound Successfully!"));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("BattleHUD: Failed to get BattleControlSubsystem!"));
		}
	}
}

void ABattleHUD::HandleBattleStateChanged(EBattleState NewState)
{
	// TODO: 전투 상태 공지 위젯 텍스트 업데이트
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
}

void ABattleHUD::HandleTurnUnitChanged(ABattleBaseUnit* ActiveUnit)
{
	if (!BattleActionMenuWidget || !ActiveUnit) return;
	
	// 아군 턴일 때만 스킬창 활성화 로직을 HUD가 직접 제어
	//bool bIsAlly = ActiveUnit->IsA<ABattleAllyUnit>();
	//BattleActionMenuWidget->SetVisibility(bIsAlly ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	
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
	if (!NewTarget) return;
}

void ABattleHUD::HandleAllyUnitInfoUpdate()
{
	
}
