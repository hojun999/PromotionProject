// Fill out your copyright notice in the Description page of Project Settings.


#include "BattleTurnOrderSlot.h"
#include "Components/Image.h"
#include "BattleBaseUnit.h"
#include "UnitDataAsset.h"

void UBattleTurnOrderSlot::SetUnitSlotInfo(class ABattleBaseUnit* TargetUnit)
{
	if (TargetUnit && IsValid(TargetUnit->UnitData))
	{
		// 데이터 에셋에 저장된 유닛의 초상화를 UI에 세팅
		UnitPortrait->SetBrushFromTexture(TargetUnit->UnitData->UnitIcon);
	}
}
