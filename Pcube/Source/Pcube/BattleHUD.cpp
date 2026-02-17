// Fill out your copyright notice in the Description page of Project Settings.


#include "BattleHUD.h"
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
	
	// 각 턴마다 유닛들의 행동 순서를 나타내는 위젯 생성
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
	
	// 스킬 패널 생성
	if (ActionMenuClass)
	{
		BattleActionMenuWidget = CreateWidget<UBattleActionMenu>(PC, ActionMenuClass);
		BattleActionMenuWidget->AddToViewport();
		BattleActionMenuWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
	
	// 전투 진행에 따른 텍스트 출력 위젯 생성
	if (BattleStateNoticeClass)
	{
		BattleStateNoticeWidget = CreateWidget<UUserWidget>(PC, BattleStateNoticeClass);
		BattleStateNoticeWidget->AddToViewport();
	}
	
	if (GameOverWidgetClass)
	{
		GameOverWidget = CreateWidget<UUserWidget>(PC, GameOverWidgetClass);
		GameOverWidget->AddToViewport(50);
		GameOverWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
	
	if (VictoryWidgetClass)
	{
		VictoryWidget = CreateWidget<UUserWidget>(PC, VictoryWidgetClass);
		VictoryWidget->AddToViewport(50);
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
