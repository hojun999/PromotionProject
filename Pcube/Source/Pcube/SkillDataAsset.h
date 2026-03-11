// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SkillDataAsset.generated.h"

class ABattleProjectile;

UENUM(BlueprintType)
enum class ESkillTargetRule : uint8
{
	SingleEnemy UMETA(DisplayName="Single Enemy"),
	AllEnemis UMETA(DisplayName="All Enemies"),
	SingleAlly UMETA(DisplayName="Single Ally"),
	AllAllies UMETA(DisplayName="All Ally"),
	Self UMETA(DisplayName="Self"),
};

UENUM(BlueprintType)
enum class ESkillEffectType : uint8
{
	Damage UMETA(DisplayName="Damage"),
	Heal UMETA(DisplayName="Heal"),
	BuffATK UMETA(DisplayName="Buff Attack"),
	Stun UMETA(DisplayName="Stun"),
};

UENUM(BlueprintType)
enum class ESkillDeliveryType : uint8
{
	Instant UMETA(DisplayName="Instant"), // 노티파이 타이밍에 즉시 ApplyDamage
	Projectile UMETA(DisplayName="Projectile") // 노티파이 타이밍에 투사체 발사, 충돌 시 ApplyDamage 
};

USTRUCT(BlueprintType)
struct FBuffAtkSpec
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AddMultiplier = 0.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AddFlat = 0.0f;
	
	// 행동 기준 n턴 - 해당 유닛이 n번 행동할 때까지 유지
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 DurationActions = 1;
};

UCLASS()
class PCUBE_API USkillDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Skill")
	FString SkillName;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Skill")
	UTexture2D* SkillImage = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Skill")
	ESkillTargetRule TargetType = ESkillTargetRule::SingleEnemy;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Skill")
	ESkillEffectType EffectType = ESkillEffectType::Damage;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Delivery")
	ESkillDeliveryType DeliveryType = ESkillDeliveryType::Instant;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Delivery", meta=(EditCondition="DeliveryType==ESkillDeliveryType::Projectile"))
	TSubclassOf<ABattleProjectile> ProjectileClass; // 투사체 액터 클래스
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Delivery", meta=(EditCondition="DeliveryType==ESkillDeliveryType::Projectile"))
	FName MuzzleSocketName = TEXT("Muzzle"); // 투사체 발사 소켓(손/무기 소켓)

	// 투사체 스폰 로컬 오프셋(소켓 기준). 손/지팡이 끝에서 살짝 앞으로 빼고 싶을 때 사용
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Delivery", meta=(EditCondition="DeliveryType==ESkillDeliveryType::Projectile"))
	FVector ProjectileSpawnLocalOffset = FVector::ZeroVector;

	// 다중 투사체일 때 겹치지 않도록 좌우로 벌리는 간격(cm)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Delivery", meta=(EditCondition="DeliveryType==ESkillDeliveryType::Projectile", ClampMin="0.0"))
	float ProjectileLateralSpacing = 8.0f;

	// 다중 투사체를 한 프레임에 다 쏘지 않고 순차 발사하는 간격(초). 0이면 동시 스폰
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Delivery", meta=(EditCondition="DeliveryType==ESkillDeliveryType::Projectile", ClampMin="0.0"))
	float ProjectileSpawnInterval = 0.05f;
	
	// Instant 타입 다중 히트 사이 시간 간격(초). 0이면 즉시 전부 적용
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Delivery", meta=(EditCondition="DeliveryType==ESkillDeliveryType::Instant", ClampMin="0.0"))
	float HitSpawnInterval = 0.15f;
	
	// --- Damage ---
	// 최종 데미지 = (Attacker ATK * DamageMultiplier) - 추가 보정은 런타임 Spec에서
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Damage")
	float DamageMultiplier = 1.0f;
	
	// hit count
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Damage")
	int32 BaseHitCount = 1;
	
	// --- Heal ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Damage")
	float HealAmount = 0.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Delivery", meta=(ClampMin="1", EditCondition="DeliveryType==ESkillDeliveryType::Projectile"))
	int32 BaseProjectileCount = 1; // 기본 투사체 수 - 부품 효과 보정 전
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Delivery", meta=(EditCondition="DeliveryType==ESkillDeliveryType::Projectile"))
	bool bSplitDamageAcrossProjectiles = true; // 투사체 수 증가 시 총 데미지 유지 여부
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Buff")
	FBuffAtkSpec BuffAtk;
	
	// --- 스킬 포인트 ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cost")
	int32 SkillPointCost = 0; // 스킬 사용 시 소모되는 값, 기본 공격은 0
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cost")
	int32 SkillPointGainOnUse = 0; // 스킬 사용 후 얻는 값, 기본 공격은 +1
	
	// --- 스턴 지속시간 ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stun")
	int32 StunDurationActions = 1; // 한 텀 쉼 = 1
	
	// --- 연출 ---
	UPROPERTY(EditAnywhere, Category="Battle|Skill")
	class UAnimMontage* ActionMontage;
	
	UFUNCTION(BlueprintCallable)
	bool RequiresTargetSelection() const
	{
		return TargetType == ESkillTargetRule::SingleEnemy
			|| TargetType == ESkillTargetRule::SingleAlly;
	};
	
	// 스킬 속성, 애니메이션, 사용 행동력 등 정의
};
