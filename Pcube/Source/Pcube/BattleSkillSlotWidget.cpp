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

	if (SkillDescriptionText)
	{
		const TCHAR* TargetLabel = TEXT("Target");
		switch (Skill->TargetType)
		{
		case ESkillTargetRule::SingleEnemy: TargetLabel = TEXT("Single Enemy"); break;
		case ESkillTargetRule::AllEnemis:   TargetLabel = TEXT("All Enemies"); break;
		case ESkillTargetRule::SingleAlly:  TargetLabel = TEXT("Single Ally"); break;
		case ESkillTargetRule::AllAllies:   TargetLabel = TEXT("All Allies"); break;
		case ESkillTargetRule::Self:        TargetLabel = TEXT("Self"); break;
		}

		const TCHAR* EffectLabel = TEXT("Effect");
		switch (Skill->EffectType)
		{
		case ESkillEffectType::Damage:  EffectLabel = TEXT("Damage"); break;
		case ESkillEffectType::Heal:    EffectLabel = TEXT("Heal"); break;
		case ESkillEffectType::BuffATK: EffectLabel = TEXT("Buff"); break;
		case ESkillEffectType::Stun:    EffectLabel = TEXT("Stun"); break;
		}

		SkillDescriptionText->SetText(FText::FromString(
			FString::Printf(TEXT("%s | %s | SP %d"), EffectLabel, TargetLabel, Skill->SkillPointCost)));
	}
}

void UBattleSkillSlotWidget::HandleClicked()
{
	if (Skill)
	{
		OnSkillClicked.Broadcast(Skill);
	}
}
