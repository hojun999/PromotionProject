// Fill out your copyright notice in the Description page of Project Settings.


#include "BattleActionOrderSlot.h"
#include "Components/Image.h"
#include "BattleBaseUnit.h"
#include "UnitDataAsset.h"

void UBattleActionOrderSlot::SetUnitSlotInfo(class ABattleBaseUnit* TargetUnit)
{
	//if (!TargetUnit || !UnitPortrait) return;
	
	if (!TargetUnit) {
		UE_LOG(LogTemp, Error, TEXT("BattleUI: TargetUnit is NULL!"));
		return;
	}
	
	if (!UnitIcon) {
		UE_LOG(LogTemp, Error, TEXT("BattleUI: UnitPortrait(Image Widget) is NOT BOUND!"));
		return;
	}
	
	// 1. 유닛으로부터 데이터 에셋 가져오기
	UUnitDataAsset* UnitData = TargetUnit->UnitData;
	if (!UnitData)
	{
		UE_LOG(LogTemp, Error, TEXT("BattleUI: UnitData is NULL for Unit: %s"), *TargetUnit->GetName());
		return;
	}
	
	// 2. 유닛 데이터로부터 이미지 가져오기
	UTexture2D* PortraitTexture = UnitData->UnitIcon;
	
	if (PortraitTexture)
	{
		// 3. 위젯의 이미지 컴포넌트에 텍스처 할당
		UnitIcon->SetBrushFromTexture(PortraitTexture);
		UE_LOG(LogTemp, Log, TEXT("BattleUI: Successfully set icon for %s"), *UnitData->UnitName);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("BattleUI: UnitIcon is NULL in DataAsset: %s"), *UnitData->UnitName);
	}
}
