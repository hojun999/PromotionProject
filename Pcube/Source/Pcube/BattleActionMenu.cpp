// Fill out your copyright notice in the Description page of Project Settings.


#include "BattleActionMenu.h"
#include "Components/VerticalBox.h"

void UBattleActionMenu::ShowMenu(ABattleAllyUnit* TargetUnit)
{
	if (!TargetUnit) return;
	CurrentUnit = TargetUnit;
	
	// 1. 유닛의 오른쪽 공간 좌표 계산 (World Space)
	// 유닛 위치에서 오른쪽 방향으로 100 유닛, 위로 50 유닛 오프셋
	FVector WorldLocation = CurrentUnit->GetActorLocation() 
							+ (CurrentUnit->GetActorRightVector() * 200.f) 
							+ (CurrentUnit->GetActorUpVector() * 1.f);
	
	// 2. 월드 좌표를 화면 2D 좌표로 투영 (Project)
	APlayerController* PC = GetOwningPlayer();
	FVector2D ScreenPosition;
    
	if (PC && PC->ProjectWorldLocationToScreen(WorldLocation, ScreenPosition))
	{
		// 3. 위젯 위치 고정 및 가시성 ON
		SetPositionInViewport(ScreenPosition);
		SetVisibility(ESlateVisibility::Visible);
        
		// 4. 메뉴 초기화 (메인 버튼 페이지 보여주기)
		//if (MenuSwitcher) MenuSwitcher->SetActiveWidgetIndex(0);
		
		// 서브 메뉴들은 일단 숨김
		SkillListWidget->SetVisibility(ESlateVisibility::Collapsed);
		ItemListWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
	
	
}

void UBattleActionMenu::OnAttackClicked()
{
	
}

void UBattleActionMenu::OnSkillMenuClicked()
{
	
}

void UBattleActionMenu::OnItemMunuClicked()
{
	
}
