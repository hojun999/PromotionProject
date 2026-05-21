// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponPartDataAsset.h"

FText UWeaponPartDataAsset::BuildEffectText() const
{
	TArray<FString> Lines;
	
	if (Effect.BonusProjectileCount != 0)
	{
		Lines.Add(FString::Printf(TEXT("투사체 %+d"), Effect.BonusProjectileCount));
	}
	if (Effect.BonusHitCount != 0)
	{
		Lines.Add(FString::Printf(TEXT("타수 %+d"), Effect.BonusHitCount));
	}
	if (Effect.BonusSkillPointGain != 0)
	{
		Lines.Add(FString::Printf(TEXT("SP 추가 획득 %d"), Effect.BonusSkillPointGain));
	}
	if (Effect.BonusMaxHP != 0)
	{
		Lines.Add(FString::Printf(TEXT("체력 추가 획득 %f.1"), Effect.BonusMaxHP));
	}
	if (Effect.BonusAttackPower != 0)
	{
		Lines.Add(FString::Printf(TEXT("공격력 추가 획득 %f.1"), Effect.BonusAttackPower));
	}
	if (Effect.BonusSpeed != 0)
	{
		Lines.Add(FString::Printf(TEXT("속도 추가 획득 %f.1"), Effect.BonusSpeed));
	}
	
	
	if (Lines.Num() == 0)
	{
		Lines.Add(TEXT("효과 없음"));
	}
	
	return FText::FromString(FString::Join(Lines, TEXT("\n")));
}
