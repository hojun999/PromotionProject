// Fill out your copyright notice in the Description page of Project Settings.


#include "BattleHUD.h"
#include "BattleAllyUnit.h"
#include "BattleBaseUnit.h"
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
	// 전투 상태 공지 위젯 텍스트 업데이트
	
	// 연출 중 UI 상호작용 제어
	if (NewState == EBattleState::ActionExecute && BattleActionMenuWidget)
	{
		BattleActionMenuWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
}

void ABattleHUD::HandleTurnUnitChanged(ABattleBaseUnit* ActiveUnit)
{
	if (!BattleActionMenuWidget || !ActiveUnit) return;
	
	// 아군 턴일 때만 스킬창 활성화 로직을 HUD가 직접 제어
	bool bIsAlly = ActiveUnit->IsA<ABattleAllyUnit>();
	BattleActionMenuWidget->SetVisibility(bIsAlly ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	
	if (ABattleAllyUnit* AllyUnit = Cast<ABattleAllyUnit>(ActiveUnit))
	{
		// 1. 카메라 줌인 (시네마틱 카메라 전환)
		APlayerController* PC = GetOwningPlayerController();
		// AllyUnit 내부의 CineCameraComponent로 0.8초간 부드럽게 전환
		PC->SetViewTargetWithBlend(AllyUnit, 0.8f, VTBlend_Cubic);
		
		if (BattleActionMenuWidget)
		{
			BattleActionMenuWidget->ShowMenu(AllyUnit);
		}
	}
	else // 적군 턴일 때 (필요시 적군에게도 카메라 포커스 가능)
	{
		
		if (BattleActionMenuWidget) BattleActionMenuWidget->SetVisibility(ESlateVisibility::Collapsed);
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

void ABattleHUD::HandleAllyUnitInfoUpdate()
{
	
}
