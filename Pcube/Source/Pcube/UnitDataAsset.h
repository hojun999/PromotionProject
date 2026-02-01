// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BattleEnemyUnit.h"
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
	
	// --- 비주얼 설정 ---
	// 예시용 스태틱 메시
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Visual")
	UStaticMesh* UnitStaticMesh;
	
	// 캐릭터용 스켈레탈 메시 (애니메이션이 필요한 경우)
	// UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Visual")
	// USkeletalMesh* UnitSkeletalMesh;
	
	// 애니메이션 블루프린트 (공격, 대기 등 동작 제어)
	// UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Visual")
	// TSubclassOf<UAnimInstance> AnimBlueprintClass;
	
	// ---
	
	// 전투용 정보 (전투 레벨에서 소환할 블루프린트 클래스)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Battle")
	TSubclassOf<ABattleEnemyUnit> BattleUnitClass;
	
	// 기본 스탯
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats")
	FUnitBaseStats BaseStats;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Skills")
	TArray<USkillDataAsset*> SkillList;
};
