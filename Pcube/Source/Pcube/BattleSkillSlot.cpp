// Fill out your copyright notice in the Description page of Project Settings.


#include "BattleSkillSlot.h"

#include "BattleAllyUnit.h"
#include "SkillDataAsset.h"
#include "UnitDataAsset.h"

void UBattleSkillSlot::SetSkillSlotInfo(class USkillDataAsset* TargetSkillDataInfo)
{
	if (!TargetSkillDataInfo)
	{
		UE_LOG(LogTemp, Error, TEXT("BattleSkillSlot: TargetUnit is NULL!"));
		return;
	}
	
	SkillDataInfo = TargetSkillDataInfo;
	
	// 스킬 아이콘 이미지 변경
	if (SkillIcon && !SkillDataInfo->SkillImage)
	{
		SkillIcon->SetBrushFromTexture(SkillDataInfo->SkillImage);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("BattleSkillSlot: SkillIcon Widget or Texture is missing."));
	}
	
	if (SkillNameText)
	{
		SkillNameText->SetText(FText::FromString(SkillDataInfo->SkillName));
	}
}
