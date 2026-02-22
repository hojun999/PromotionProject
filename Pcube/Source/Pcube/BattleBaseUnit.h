// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CineCameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "SkillDataAsset.h"
#include "UnitDataAsset.h"
#include "BattleBaseUnit.generated.h"

USTRUCT()
struct FActiveAtkBuff
{
	GENERATED_BODY()
	
	UPROPERTY()
	float AddMultiplier = 0.f;
	
	UPROPERTY()
	float AddFlat = 0.f;
	
	UPROPERTY()
	int32 RemainingActions = 0;
};

USTRUCT()
struct FSkillRuntimeModifier
{
	GENERATED_BODY()
	
	UPROPERTY()
	int32 BonusHitCount = 0; // 부품 시스템 - 타수 증가
	
	UPROPERTY()
	int32 BonusProjectileCount = 0; // 부품 시스템 - 투사체 수 증가
	
	UPROPERTY()
	float DamageMulAdd = 0.f; 
	
	UPROPERTY()
	float DamageMulMul = 1.f;
};

USTRUCT()
struct FSkillRuntimeSpec
{
	GENERATED_BODY()
	
	UPROPERTY()
	ESkillTargetRule TargetType = ESkillTargetRule::SingleEnemy;
	
	UPROPERTY()
	ESkillEffectType EffectType = ESkillEffectType::Damage;
	
	UPROPERTY()
	float DamageMultiplier = 1.f;
	
	UPROPERTY()
	int32 Hitcount = 1; // 타격 횟수
	
	// --- projectile ---
	UPROPERTY()
	ESkillDeliveryType DeliveryType = ESkillDeliveryType::Instant; // 즉발/투사체 분기
	
	UPROPERTY()
	TSubclassOf<class ABattleProjectile> ProjectileClass = nullptr;
	
	UPROPERTY()
	FName MuzzleSocketName = NAME_None; // 발사 소켓
	
	UPROPERTY()
	int32 ProjectileCount = 1; // 웨이브당 투사체 개수
	
	UPROPERTY()
	bool bSplitDamageAcrossProjectiles = true; // 투사체당 대미지 분배 여부
	
	// --- 기타 ---
	UPROPERTY()
	float HealAmount = 0.f;
	
	UPROPERTY()
	FBuffAtkSpec BuffAtk;
	
	UPROPERTY()
	int32 StunDurationActions = 0;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnActionFinished);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHPChanged, float, CurrentHP, float, MaxHP);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnUnitDied);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSkillPointsChanged, int32, Current, int32, Max);

UCLASS()
class PCUBE_API ABattleBaseUnit : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ABattleBaseUnit();

	// 유닛의 데이터 에셋 초기화를 담당하는 함수
	virtual void InitUnit(UUnitDataAsset* TransferredUnitData);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	UUnitDataAsset* UnitData;
	
	// 아군: 파티에서의 INDEX를 의미, 적: INDEX_NONE 유지
	UPROPERTY(BlueprintReadOnly, Category="Persistent")
	int32 PartyIndex = INDEX_NONE;
	
	// 스킬 목록 (아군/적 개수 제한 없이 데이터로 관리)
	UPROPERTY(EditAnywhere, Category = "Skills")
	TArray<USkillDataAsset*> Skills;
	
	// --- 스탯 ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Status")
	float CurrentHP;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Status")
	float CurrentSpeed;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Status")
	float CurrentAttackPower;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Status")
	int32 StunRemainingActions = 0;
	
	UFUNCTION(BlueprintCallable)
	bool IsStunned() const { return StunRemainingActions > 0; }
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="SkillPoints")
	int32 CurrentSkillPoints = 0;
	
	UPROPERTY(BlueprintReadOnly)
	bool bIsDead = false;
	
	UFUNCTION(BlueprintCallable)
	float GetMaxHP() const;
	
	UFUNCTION(BlueprintCallable)
	bool IsDead() const { return bIsDead || CurrentHP <= 0.f; }
	
	UFUNCTION(BlueprintCallable)
	int32 GetCurrentSkillPoints() const { return CurrentSkillPoints; }
	
	UFUNCTION(BlueprintCallable)
	int32 GetMaxSkillPoints() const { return UnitData ? FMath::Max(0, UnitData->MaxSkillPoints) : 0; }
	
	UFUNCTION(BlueprintCallable)
	int32 GetSkillNumber(); // 유닛의 보유 스킬 개수를 구하는 함수
	
	bool CanUseSkill(const USkillDataAsset* Skill) const;
	bool SpendSkillPoints(int32 Cost);
	void GainSkillPoints(int32 Amount);
	
	// 델리게이트 인스턴스 변수 생성
	UPROPERTY(BlueprintAssignable, Category="Battle")
	FOnActionFinished OnActionFinished;
	
	UPROPERTY(BlueprintAssignable, Category="Battle|Event")
	FOnHPChanged OnHPChanged;
	
	UPROPERTY(BlueprintAssignable, Category="Battle|Event")
	FOnUnitDied OnUnitDied;
	
	UPROPERTY(BlueprintAssignable, Category="Battle|Event")
	FOnSkillPointsChanged OnSkillPointsChanged;
	
	// 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="UI")
	class USceneComponent* UIAnchorPoint;
	
	// --- 전투 핵심 로직
	void FinishAction(); // 행동 종료 시 호출할 함수
	void ExecuteAction(USkillDataAsset* SkillData, AActor* TargetUnit);
	void ExecuteAction(USkillDataAsset* SkillData, const TArray<AActor*>& Targets);
	void ExecuteActionOnTargets(USkillDataAsset* SkillData, const TArray<ABattleBaseUnit*>& Targets);
	void ApplyCurrentSkillDamage();
	void ApplyStun(int32 DurationActions);
	
	UFUNCTION(BlueprintCallable)
	void Heal(float Amount);
	
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	virtual  void BasicAttack(ABattleBaseUnit* Target); // 공통 일반 공격 함수
	virtual  void Die(); // 사망 처리 함수
	
	// --- 투사체 ---
	void NotifyProjectileResolved(ABattleBaseUnit* HitTarget, float Damage, bool bDidHit);
	
	// UFUNCTION(BlueprintCallable)
	// float GetHPPercent() const
	// {
	// 	return 
	// }
	
	// --- 스펙(스탯) ---
	UPROPERTY()
	FSkillRuntimeSpec CurrentActionSpec; // 액션 시작 시 확정된 스킬 파라미터(타수/배율/효과)를 노티파이 동안 일관되게 사용
	
	// --- 노티파이 ---
	// 노티파이가 다른 몽타주에서 섞여 들어오는 걸 막기 위한 필터
	UPROPERTY()
	int32 CurrentMontageInstanceID = INDEX_NONE;	// 이번 액션에서 재생한 몽타주 인스턴스 ID(NotifyPayload의 MontageInstanceID와 비교)
	
	// 문자열 비교 실수 방지용 상수
	static const FName Notify_Hit; // 타격/효과 1회 적용 트리거
	static const FName Notify_Effect; // 한 번에 전부 적용 같은 확장 트리거
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Visual")
	UStaticMeshComponent* StaticMeshComp;
	
	// UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
	// USpringArmComponent* CineCameraArmComp;
	// UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
	// UCineCameraComponent* CineCameraComp;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
	USpringArmComponent* CameraArmComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
	UCameraComponent* CameraComp;
	
	// 애니메이션 노티파이로부터 호출될 함수
	UFUNCTION()
	void HandleHitNotify(FName NotifyName, const FBranchingPointNotifyPayload& Payload);
	
	UFUNCTION()
	void OnActionMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	
	// 마우스 클릭 시 호출되는 엔진 기본 이벤트
	virtual void NotifyActorOnClicked(FKey ButtonPressed = EKeys::LeftMouseButton) override;
	
	// 선택 상태 관리 변수
	UPROPERTY(BlueprintReadOnly, Category="Selection")
	bool bIsSelected;
	
private:
	UPROPERTY()
	ABattleBaseUnit* CurrentActionTarget;	// 액션 시작 시 확정된 스킬 스펙(노티파이마다 재계산 금지)
	
	UPROPERTY()
	TArray<TWeakObjectPtr<ABattleBaseUnit>> CurrentActionTargets;
	
	UPROPERTY()
	class USkillDataAsset* CurrentSkillData; // 현재 사용 중인 스킬 데이터

	UPROPERTY()
	bool bDamageAppliedThisAction = false;
	
	UPROPERTY()
	int32 ActionTotalHits = 1;
	
	UPROPERTY()
	int32 ActionHitsApplied = 0;
	
	// --- 애니메이션/몽타주 ---
	UPROPERTY()
	bool bActionMontageEnded = false; // 몽타주 종료 여부 (투사체 대기 시 FinishAction 지연용)
	
	void TryFinishAction(); // 몽타주 종료 + 투사체 전부 종료면 FinishAction 호출
	
	// --- 투사체 ---
	UPROPERTY()
	int32 PendingProjectiles = 0; // 아직 충돌/소멸되지 않은 투사체 수
	
	void SpawnProjectileVolley(const TArray<ABattleBaseUnit*>& Targets, float WaveDamage);
	
	// --- 버프 ---
	UPROPERTY()
	TArray<FActiveAtkBuff> ActiveAtkBuffs;
	
	FSkillRuntimeModifier GetSkillRuntimeModifier(const USkillDataAsset* Skill) const; // 부품 시스템 훅
	FSkillRuntimeSpec BuildRuntimeSpec(const USkillDataAsset* Skill) const;
	
	//float GetMaxHP() const;
	float GetEffectiveAttackPower() const;
	
	static void ApplyHealToUnit(ABattleBaseUnit* Target, float HealAmount);
	
	void ApplyOneHit(); // 1타 or 1회 효과 적용
	void ApplyRemainingHits(); // 남은 타수 일괄 적용
	void TickBuffDuration_OnActionEnd(); // 본인 행동 종료시 지속턴 감소
	
};
