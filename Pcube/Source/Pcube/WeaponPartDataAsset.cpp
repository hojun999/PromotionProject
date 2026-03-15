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
	
	if (Lines.Num() == 0)
	{
		Lines.Add(TEXT("효과 없음"));
	}
	
	return FText::FromString(FString::Join(Lines, TEXT("\n")));
}
