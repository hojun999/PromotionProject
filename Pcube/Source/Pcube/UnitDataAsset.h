// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponDataAsset.h"
#include "Engine/DataAsset.h"
#include "UnitDataAsset.generated.h"

class ABattleBaseUnit;
class USkillDataAsset;
class UAnimMontage;
class UParticleSystem;

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

	// --- 장비(고정 장비) ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Equipment|Weapon")
	TObjectPtr<UWeaponDataAsset> DefaultWeapon = nullptr;
	
	// 캐릭터 스켈레탈 메시에 추가한 소켓 (예: r_hand_socket에 무기(SM) 부착)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Equipment|Weapon")
	FName WeaponAttachSocketName = NAME_None;
	
	// 피격 몽타주(데미지 받을 때 재생) - 아군 등 스켈레탈 유닛용
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Visual|Battle")
	TObjectPtr<UAnimMontage> HitReactMontage = nullptr;
	
	// 스킬이 공용(예: BasicAttackData)이어도 유닛마다 다른 몽타주를 재생하기 위한 오버라이드
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Visual|Battle")
	TMap<USkillDataAsset*, UAnimMontage*> SkillMontageOverrides;
	

	// 투사체 스킬이 공용이어도 유닛마다 다른 발사 소켓을 쓰고 싶을 때 오버라이드
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Visual|Battle")
	TMap<USkillDataAsset*, FName> SkillMuzzleSocketOverrides;

	// 적 유닛이 "공격 몽타주가 없을 때" 타격이 너무 즉시 들어가는 문제를 완화하기 위한 기본 지연
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Battle|Timing")
	float EnemyNoMontageAttackDelaySeconds = 1.5f;
	
	// 적 피격 VFX(캐스케이드 파티클). 적이 스태틱/스켈레탈이 섞여 있어도 위치 기반으로 스폰 가능하게 구성
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Visual|Battle|React")
	TObjectPtr<UParticleSystem> EnemyHitParticle = nullptr;

	// 소켓이 존재하면 소켓 기준, 없으면 Offset 기준으로 스폰
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Visual|Battle|React")
	FName EnemyHitParticleSocket = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Visual|Battle|React")
	FVector EnemyHitParticleOffset = FVector(0.f, 0.f, 80.f);
	
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
