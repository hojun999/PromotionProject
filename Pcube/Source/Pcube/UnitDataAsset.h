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
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="UI")
	TObjectPtr<UTexture2D> PortraitTexture = nullptr;
	
	// --- 비주얼 설정 ---
	// 예시용 스태틱 메시
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Visual|Battle")
	UStaticMesh* UnitStaticMesh;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Visual|Battle")
	USkeletalMesh* BattleSkeletalMesh;
	
	// 캐릭터용 스켈레탈 메시 (애니메이션이 필요한 경우)
	// 아군 - 기본, 적 - 살아있을 때
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Visual|World")
	USkeletalMesh* WorldSkeletalMesh;
	
	// 애니메이션 블루프린트 (공격, 대기 등 동작 제어)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Visual|World")
	TSubclassOf<UAnimInstance> WorldAnimBlueprintClass;
	
	// 적 전용 - 월드에서 죽었을 때 전환할 스태틱 메시
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Visual|World")
	UStaticMesh* DeadStaticMesh;
	
	// ---
	
	// 전투용 정보 (전투 레벨에서 소환할 블루프린트 클래스)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Battle")
	TSubclassOf<ABattleBaseUnit> BattleUnitClass;
	
	// L_Battle에서 사용할 AnimBP(Idle/피격/사망 기본 StateMachine 포함)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Visual|Battle")
	TSubclassOf<UAnimInstance> BattleAnimBlueprintInstance = nullptr;
	
	// 기본 스탯
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats")
	FUnitBaseStats BaseStats;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SkillPoints")
	int32 BaseSkillPoints = 0; // 아군 유닛마다의 값을 가짐. 에디터에서 설정
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SkillPoints")
	int32 MaxSkillPoints; // 적: 0으로 설정, SP 시스템 미사용
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Skills")
	TArray<USkillDataAsset*> SkillList;
};
