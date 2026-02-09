// Fill out your copyright notice in the Description page of Project Settings.


#include "BattleActionMenu.h"

#include "Components/VerticalBox.h"

void UBattleActionMenu::ShouMenu(ABattleAllyUnit* TargetUnit)
{
	CurrentUnit = TargetUnit;
	this->SetVisibility(ESlateVisibility::Visible);
	
	// 서브 메뉴들은 일단 숨김
	SkillListWidget->SetVisibility(ESlateVisibility::Collapsed);
	ItemListWidget->SetVisibility(ESlateVisibility::Collapsed);
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
