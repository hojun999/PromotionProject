// Fill out your copyright notice in the Description page of Project Settings.


#include "BattleHUDWidget.h"

#include "BattleAllyUnit.h"
#include "Components/CanvasPanel.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"

// 유닛 스폰이 끝난 후, "Battle Start" 텍스트 출력
// 유닛 스폰이 끝난 후, 아군 초상화 및 스탯 UI
// BattleControlSubSystem에서 유닛 스폰이 끝난

// 각 턴이 시작될 때, 갱신된 유닛 행동 순서 UI

// 특정 유닛 턴 때
// 아군인 경우, 일반 공격, 스킬 공격, 아이템 사용의 3가지 버튼 UI
// 적인 경우, X

// 적이 전멸한 경우, "Battle End" 텍스트 출력

// 아군이 전멸한 경우, "You Die" 텍스트 출력

void UBattleHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (UWorld* World =GetWorld())
	{
		// BattleControlSubsystem 가져오기
		UBattleControlSubsystem* BattleSubsystem = World->GetSubsystem<UBattleControlSubsystem>();
		if (BattleSubsystem)
		{
			// BattleControlSubsystem의 델리게이트 구독
			BattleSubsystem->OnBattleStateChanged.AddDynamic(this, &UBattleHUDWidget::OnBattleStateChanged);
			BattleSubsystem->OnTurnUnitChanged.AddDynamic(this, &UBattleHUDWidget::OnTurnUnitChagned);
		}
	}
	
}

void UBattleHUDWidget::OnBattleStateChanged(EBattleState NewState)
{
	UpdateUIState(NewState);
	
	// 상태에 따른 텍스트 연출
	FString StateMsg;
	switch (NewState)
	{
	case EBattleState::NewRound:
		StateMsg = TEXT("Player Turn"); break;
	case EBattleState::CheckCondition:
		StateMsg = TEXT("Battle End"); break;
		default:
		TEXT(""); break;
	}
	
	if (StateNoticeText)
	{
		StateNoticeText->SetText((FText::FromString(StateMsg)));
	}
}

void UBattleHUDWidget::OnTurnUnitChagned(ABattleBaseUnit* ActiveUnit)
{
	if (!ActiveUnit)
	{
		return;
	}
	
	// 아군 유닛인 경우에만 스킬 창 활성화
	if(SkillPanel)
	{
		bool bIsAlly = ActiveUnit->IsA<ABattleAllyUnit>();
		SkillPanel->SetVisibility(bIsAlly ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}

void UBattleHUDWidget::UpdateUIState(EBattleState State)
{
	// ActionExecute(연출 중) 상태일 때는 모든 입력 UI를 감추는 등의 처리
	if (State == EBattleState::ActionExecute)
	{
		if (SkillPanel)
		{
			SkillPanel->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
	}
}

void UBattleHUDWidget::RefreshTurnOrderList(const TArray<AActor*>& TurnOrder)
{
	if (!UnitActionOrderSlots || !TurnSlotClass)
	{
		return;
	}
	
	// 1. 기존 리스트 초기화
	UnitActionOrderSlots->ClearChildren();
	
	// 2. 전달받은 유닛 순서대로 슬롯 생성
	for (AActor* Actor : TurnOrder)
	{
		if (ABattleBaseUnit* Unit = Cast<ABattleBaseUnit>(Actor))
		{
			// 위젯 생성
			UBattleTurnOrderSlot* NewSlot = CreateWidget<UBattleTurnOrderSlot>(this, TurnSlotClass);
			if (NewSlot)
			{
				// 유닛 정보 전달
				NewSlot->SetUnitSlotInfo(Unit);
				
				// VerticalBox의 자식으로 추가
				UnitActionOrderSlots->AddChildToVerticalBox(NewSlot);
			}
		}
	}
}
