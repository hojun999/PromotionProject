// Fill out your copyright notice in the Description page of Project Settings.


#include "BattleSkillSlotWidget.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "SkillDataAsset.h"

void UBattleSkillSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (Btn_Select)
	{
		Btn_Select->OnClicked.RemoveDynamic(this, &UBattleSkillSlotWidget::HandleClicked);
		Btn_Select->OnClicked.AddDynamic(this, &UBattleSkillSlotWidget::HandleClicked);
	}
}

void UBattleSkillSlotWidget::Init(USkillDataAsset* InSkill, int32 InSkillIndex)
{
	Skill = InSkill;
	SkillIndex = InSkillIndex;
	
	if (!Skill) return;
	
	
	if (SkillNameText)
	{
		SkillNameText->SetText(FText::FromString(
			FString::Printf(TEXT("%d. %s"), SkillIndex, *Skill->SkillName)
			));
	}
}

void UBattleSkillSlotWidget::HandleClicked()
{
	if (Skill)
	{
		OnSkillClicked.Broadcast(Skill);
	}
}
