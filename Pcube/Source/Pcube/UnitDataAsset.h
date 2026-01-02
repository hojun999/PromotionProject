// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "UnitDataAsset.generated.h"

class ABattleBaseUnit;
class USkillDataAsset;

USTRUCT(BlueprintType)
struct FUnitBaseStats
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxHP = 100.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Speed = 100.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AttackPower = 10.f;
};

/**
 * 
 */
UCLASS()
class PCUBE_API UUnitDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	// 유닛 식별 정보
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Identity")
	FString UnitName;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Identity")
	UTexture2D* UnitIcon;
	
	// 전투용 정보 (전투 레벨에서 소환할 블루프린트 클래스)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Battle")
	TSubclassOf<ABattleBaseUnit> BattleUnitClass;
	
	// 기본 스탯
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats")
	FUnitBaseStats BaseStats;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Skills")
	TArray<USkillDataAsset*> SkillList;
};
