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
		Lines.Add(FString::Printf(TEXT("공격 횟수 %+d"), Effect.BonusHitCount));
	}
	if (!FMath::IsNearlyZero(Effect.DamageMulAdd))
	{
		Lines.Add(FString::Printf(TEXT("데미지 배율 %+0.2f"), Effect.DamageMulAdd));
	}
	if (!FMath::IsNearlyEqual(Effect.DamageMulMul, 1.f))
	{
		Lines.Add(FString::Printf(TEXT("데미지 배율 x%0.2f"), Effect.DamageMulMul));
	}
	
	if (Lines.Num() == 0)
	{
		Lines.Add(TEXT("효과 없음"));
	}
	
	return FText::FromString(FString::Join(Lines, TEXT("\n")));
}
