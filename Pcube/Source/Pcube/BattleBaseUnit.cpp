// Fill out your copyright notice in the Description page of Project Settings.


#include "BattleBaseUnit.h"
#include "SkillDataAsset.h"
#include "UnitDataAsset.h"
#include "Kismet/GameplayStatics.h"
#include "BattleProjectile.h"

class APlayerState;

const FName ABattleBaseUnit::Notify_Hit(TEXT("Hit"));
const FName ABattleBaseUnit::Notify_Effect(TEXT("Effect"));

// Sets default values
ABattleBaseUnit::ABattleBaseUnit()
{
	StaticMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComp"));
	StaticMeshComp->SetupAttachment(RootComponent);
	
	// ACharacter를 상속받으므로 깁본 skeletalmesh 숨기기
	// TODO: 이후에 Skeletalmesh 사용할 때 아래 내용 삭제
	// if (GetMesh())
	// {
	// 	GetMesh()->SetHiddenInGame(true);
	// }
	
	// CineCameraArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("CineCameraArmComp"));
	// CineCameraArmComp->SetupAttachment(RootComponent);
	// CineCameraArmComp->TargetArmLength = 800.0f;
	// CineCameraArmComp->SetRelativeRotation(FRotator(-30.0f, 45.0f, 0.0f));
	// CineCameraArmComp->bDoCollisionTest = false;
	// CineCameraArmComp->bUsePawnControlRotation = false;
	//
	// CineCameraComp = CreateDefaultSubobject<UCineCameraComponent>(TEXT("CineCameraComp"));
	// CineCameraComp->SetupAttachment(CineCameraArmComp, USpringArmComponent::SocketName);
	// CineCameraComp->CurrentFocalLength = 50.0f;
	
	CameraArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraArmComp"));
	CameraArmComp->SetupAttachment(RootComponent);
	CameraArmComp->TargetArmLength = 800.0f;
	CameraArmComp->SetRelativeRotation(FRotator(-30.0f, 45.0f, 0.0f));
	CameraArmComp->bDoCollisionTest = false;
	CameraArmComp->bUsePawnControlRotation = false;
	
	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->SetupAttachment(CameraArmComp, USpringArmComponent::SocketName);
	
	UIAnchorPoint = CreateDefaultSubobject<USceneComponent>(TEXT("UIAnchorPoint"));
	UIAnchorPoint->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void ABattleBaseUnit::BeginPlay()
{
	Super::BeginPlay();
	
}

// BattleContolSubsystem에서 유닛을 spanw시킨 직후 호출
void ABattleBaseUnit::InitUnit(UUnitDataAsset* TransferredUnitData)
{
	if (!TransferredUnitData) return;
	
	// --- 스탯 초기화 ---
	UnitData = TransferredUnitData;
		
	const float MaxHP = GetMaxHP();
	const float Speed = (UnitData->BaseStats.Speed > 0.f) ? UnitData->BaseStats.Speed : 1.f;
	const float ATK = (UnitData->BaseStats.AttackPower > 0.f) ? UnitData->BaseStats.AttackPower : 1.f;
	
	CurrentHP = MaxHP;
	CurrentSpeed = Speed;
	CurrentAttackPower = ATK;
	bIsDead = false;
		
	UE_LOG(LogTemp, Warning, TEXT("[InitUnit] %s HP=%.1f (MaxHP=%.1f) SPD=%.1f ATK=%.1f UnitData=%s"),
		*GetName(), CurrentHP, UnitData->BaseStats.MaxHP, CurrentSpeed, CurrentAttackPower, *GetNameSafe(UnitData));
	
	
	// --- 스킬포인트  관련 ---
	const int32 MaxSP = GetMaxSkillPoints();
	CurrentSkillPoints = (UnitData && MaxSP > 0)
		? FMath::Clamp(UnitData->BaseSkillPoints, 0, MaxSP)
		: 0;
	
	OnSkillPointsChanged.Broadcast(CurrentSkillPoints, MaxSP);
	
	// 아군/스켈레탈 기반 유닛
	if (IsValid(UnitData->BattleSkeletalMesh))
	{
		if (USkeletalMeshComponent* SkelComp = GetMesh())
		{
			SkelComp->SetSkeletalMesh(UnitData->BattleSkeletalMesh);
			
			// 전투용 AnimBP 저용 -> Idle은 AnimBP가 자동으로 재생
			if (UnitData->BattleAnimBlueprintInstance)
			{
				SkelComp->SetAnimationMode((EAnimationMode::AnimationBlueprint)); // AnimBP 구동 모드
				SkelComp->SetAnimInstanceClass(UnitData->BattleAnimBlueprintInstance); // 전투 AnimBP 부착
			}
			
			// UI 초기 갱신용 브로드캐스트
			OnHPChanged.Broadcast(CurrentHP, MaxHP);
			return;
		}
	} // TODO: 적군도 스켈레탈 메쉬로 변경
	else if (IsValid(UnitData->UnitStaticMesh))// 적군의 경우
	{
		if (StaticMeshComp)
		{
			StaticMeshComp->SetStaticMesh((UnitData->UnitStaticMesh));
		
			// 여기서 스케일이나 머티리얼 추가 조정 가능
			return;
		}
	}
	
	
}

void ABattleBaseUnit::FinishAction()
{
	UE_LOG(LogTemp, Log, TEXT("%s 유닛 행동 종료!"), *GetName());
	
	TickBuffDuration_OnActionEnd();
	
	// 턴 종료 후 SP 획득
	if (CurrentSkillData)
	{
		GainSkillPoints(CurrentSkillData->SkillPointGainOnUse);
	}
	
	// 다음 턴에 이전 스킬 잔류로 포인트가 또 들어가는 문제 방지
	CurrentSkillData = nullptr;
	CurrentActionTarget = nullptr;
	CurrentActionTargets.Empty();
	
	// 신호 발송 - 구독하고 있는 모든 곳에 알림
	if (OnActionFinished.IsBound())
	{
		OnActionFinished.Broadcast();
	}
}

void ABattleBaseUnit::TryFinishAction()
{
	// 아직 몽타주가 안 끝났으면 FinishAction 금지 - 연출 중
	if (!bActionMontageEnded) return;
	
	// 투사체가 남아 있으면 FinishAction 금지 - 충돌/소멸 대기
	if (PendingProjectiles > 0) return;
	
	// 연출 종료 & 모든 투사체 종료된 상태
	FinishAction();
}

void ABattleBaseUnit::BasicAttack(ABattleBaseUnit* Target)
{
	if (!Target || CurrentAttackPower <= 0.f) return;
	
	const float Damage = FMath::Max(1.f, CurrentAttackPower);
	
	UE_LOG(LogTemp, Warning, TEXT("[BasicAttack] %s -> %s : %0.1f"),
		*GetName(), *Target->GetName(), Damage);
	
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Yellow,
			FString::Printf(TEXT("[BasicAttack] %s -> %s : %0.1f"), *GetName(), *Target->GetName(), Damage));
	}
	
	UGameplayStatics::ApplyDamage(Target, Damage, GetController(), this, UDamageType::StaticClass());
}

void ABattleBaseUnit::ExecuteAction(USkillDataAsset* SkillData, AActor* Target)
{
	ABattleBaseUnit* T = Cast<ABattleBaseUnit>(Target);
	
	if (!IsValid(T))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Action] Invalid single target -> FinishAction"));
		FinishAction();
		return;
	}
	
	TArray<ABattleBaseUnit*> Targets;
	Targets.Add(T);
	
	ExecuteActionOnTargets(SkillData, Targets);
}

void ABattleBaseUnit::ExecuteAction(USkillDataAsset* SkillData, const TArray<AActor*>& Targets)
{
	TArray<ABattleBaseUnit*> UnitTargets;
	UnitTargets.Reserve(Targets.Num());
	
	for (AActor* A : Targets)
	{
		ABattleBaseUnit* U = Cast<ABattleBaseUnit>(A);
		if (IsValid(U) && !U->IsDead())
		{
			UnitTargets.Add(U);
		}
	}
	
	if (UnitTargets.Num() <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Action] Targets empty/invalid -> FinishAction"));
		FinishAction();
		return;
	}
	
	ExecuteActionOnTargets(SkillData, UnitTargets);
}

void ABattleBaseUnit::ExecuteActionOnTargets(USkillDataAsset* SkillData, const TArray<ABattleBaseUnit*>& Targets)
{
	if (!IsValid(SkillData))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Action] SkillData null -> FinishAction"));
		FinishAction();
		return;
	}
	
	// 포인트 소모 체크&처리
	if (!SpendSkillPoints(SkillData->SkillPointCost))
	{
		UE_LOG(LogTemp, Warning, TEXT("[SkillPoint] Not enough points. unit=%s cost=%d"),
			*GetName(), SkillData->SkillPointCost);
		FinishAction();
		return;
	}
	
	CurrentSkillData = SkillData;
	
	// 런타임 스킬 스탯 확정 - 타수/효과
	CurrentActionSpec = BuildRuntimeSpec(SkillData); // 액션의 진짜 타수/투사체수/배율 확정
	ActionTotalHits = CurrentActionSpec.Hitcount;
	ActionHitsApplied = 0;
	
	CurrentActionTargets.Empty();
	for (ABattleBaseUnit* T : Targets)
	{
		if (IsValid(T) && !T->IsDead())
		{
			CurrentActionTargets.Add(T);
		}
	}
	
	if (CurrentActionTargets.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Action] No valid targets after filter -> FinishAction"));
		FinishAction();
		return;
	}
	
	// 대표 타겟 - 카메라/로그용
	// CurrentActionTarget = CurrentActionTargets[0].Get();
	
	// 애니메이션 없으면 즉시 처리 - 프로토타입
	UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
	if (!AnimInstance || !SkillData->ActionMontage)
	{
		ApplyRemainingHits();
		FinishAction();
		return;
	}
	
	// 노티파이 바인딩
	AnimInstance->OnPlayMontageNotifyBegin.RemoveDynamic(this, &ABattleBaseUnit::HandleHitNotify);
	AnimInstance->OnPlayMontageNotifyBegin.AddDynamic(this, &ABattleBaseUnit::HandleHitNotify);
	
	// 몽타주 재생
	const float PlayResult = AnimInstance->Montage_Play(SkillData->ActionMontage);
	if (PlayResult <= 0.f)
	{
		ApplyRemainingHits();
		FinishAction();
		return;
	}
	
	// 가장 최근에 재생된 몽타주의 인스턴스 ID를 확보해서 NotifyPayload에서 들어오는 ID와 매칭
	if (FAnimMontageInstance* AMInst = AnimInstance->GetActiveInstanceForMontage(SkillData->ActionMontage))
	{
		CurrentMontageInstanceID = AMInst->GetInstanceID(); // NotifyPayload.MontageInstanceId와 비교할 값
	}
	
	// 종료 델리게이트
	FOnMontageEnded EndDelegate;
	EndDelegate.BindUObject(this, &ABattleBaseUnit::OnActionMontageEnded);
	AnimInstance->Montage_SetEndDelegate(EndDelegate, SkillData->ActionMontage);
}

void ABattleBaseUnit::HandleHitNotify(FName NotifyName, const FBranchingPointNotifyPayload& Payload)
{
	// 다른 몽타주의 Notify가 섞여 들어오면 무시
	if (CurrentMontageInstanceID != INDEX_NONE && Payload.MontageInstanceID != CurrentMontageInstanceID)
	{
		return;
	}
	
	if (NotifyName == Notify_Hit)
	{
		ApplyOneHit(); // 다타면 Hit 노티파이를 여러 개 박는 방식
	}
	else if (NotifyName == Notify_Effect)
	{
		ApplyRemainingHits(); // 한 번에 적용하는 방식으로 확장하고 싶을 때 사용
	}
}

bool ABattleBaseUnit::CanUseSkill(const USkillDataAsset* Skill) const
{
	if (!IsValid(Skill)) return false;
	
	const int32 Max = GetMaxSkillPoints();
	
	if (Max <= 0) return Skill->SkillPointCost <= 0;
	
	return CurrentSkillPoints >= Skill->SkillPointCost;
}

bool ABattleBaseUnit::SpendSkillPoints(int32 Cost)
{
	if (Cost <= 0) return true;
	
	const int32 Max = GetMaxSkillPoints();
	if (Max <= 0) return false;
	
	if (CurrentSkillPoints < Cost) return false;
	
	CurrentSkillPoints = FMath::Clamp(CurrentSkillPoints - Cost, 0, Max);
	OnSkillPointsChanged.Broadcast(CurrentSkillPoints, Max);
	return true;
}

void ABattleBaseUnit::GainSkillPoints(int32 Amount)
{
	if (Amount == 0) return;
	
	const int32 Max = GetMaxSkillPoints();
	if (Max <= 0) return;
	
	CurrentSkillPoints = FMath::Clamp(CurrentSkillPoints + Amount, 0, Max);
	OnSkillPointsChanged.Broadcast(CurrentSkillPoints, Max);
}

void ABattleBaseUnit::ApplyCurrentSkillDamage()
{
	if (!CurrentSkillData || !CurrentActionTarget) return;
	
	const int32 FinalHitCount = FMath::Max(1, CurrentSkillData->BaseHitCount);
	const float DamagePerHit = FMath::Max(1.f, CurrentAttackPower * CurrentSkillData->DamageMultiplier);
	
	// 타겟 배열이 비어있으면 기존 단일 타겟 fallback
	if (CurrentActionTargets.Num() == 0 && CurrentActionTarget)
	{
		CurrentActionTargets.Add(CurrentActionTarget);
	}
	
	for (TWeakObjectPtr<ABattleBaseUnit> TPtr : CurrentActionTargets)
	{
		ABattleBaseUnit* TargetUnit = TPtr.Get();
		if (!IsValid(TargetUnit) || TargetUnit->IsDead()) continue;
		
		for (int32 i = 0; i < FinalHitCount; ++i)
		{
			UE_LOG(LogTemp, Warning, TEXT("[Damage] %s -> %s : %.1f (hit %d/%d)"),
				*GetName(), *TargetUnit->GetName(), DamagePerHit, i + 1, FinalHitCount);

			UGameplayStatics::ApplyDamage(TargetUnit, DamagePerHit, GetController(), this, UDamageType::StaticClass());
		}
	}
}

void ABattleBaseUnit::ApplyStun(int32 DurationActions)
{
	if (DurationActions <= 0) return;
	
	// 중첩이 아니라 갱신/연장으로 처리
	StunRemainingActions = FMath::Max(StunRemainingActions, DurationActions);
	
	UE_LOG(LogTemp, Warning, TEXT("[Stun] %s stunned. remainingActions=%d"),
		*GetName(), StunRemainingActions);
}

void ABattleBaseUnit::OnActionMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr)
	{
		AnimInstance->OnPlayMontageNotifyBegin.RemoveDynamic(this, &ABattleBaseUnit::HandleHitNotify);
	}

	// 노티파이가 부족했으면 남은 웨이브 실행 (투사체 추가 발사 가능)
	ApplyRemainingHits();
	
	bActionMontageEnded = true; // 몽타주 종료 신호
	TryFinishAction(); // 투사체 남아있으면 여기서 안끝남
}

float ABattleBaseUnit::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	
	const float MaxHP = GetMaxHP();
	CurrentHP = FMath::Clamp(CurrentHP - ActualDamage,  0.f,  MaxHP);
	
	UE_LOG(LogTemp, Warning, TEXT("%s took %f damage. Remaining HP: %f/%f"), *GetName(), ActualDamage, CurrentHP, MaxHP);
	
	// UI 갱신
	OnHPChanged.Broadcast(CurrentHP, MaxHP);
	
	if (CurrentHP <= 0.f)
	{
		Die();
	}

	return ActualDamage;
}

void ABattleBaseUnit::Heal(float Amount)
{
	if (bIsDead) return;
	if (Amount <= 0.0f) return;
	
	const float MaxHP = GetMaxHP();
	CurrentHP = FMath::Clamp(CurrentHP + Amount,  0.f,  MaxHP);
	
	OnHPChanged.Broadcast(CurrentHP, MaxHP);
}


void ABattleBaseUnit::Die()
{
	if (bIsDead) return;
	bIsDead = true;
	
	UE_LOG(LogTemp, Error, TEXT("BattleBaseUnit: %s has died."), *GetName());
	
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow,
			FString::Printf(TEXT("[Death] %s died."), *GetName()));
	}
	
	// 전투 중 사망한 유닛에 대한 클릭/타겟팅/충돌 방지
	SetActorEnableCollision(false);
	
	OnUnitDied.Broadcast();
}

void ABattleBaseUnit::NotifyProjectileResolved(ABattleBaseUnit* HitTarget, float Damage, bool bDidHit)
{
	// 대미지/상태이상 적용은 충돌 성공했을 대만
	if (bDidHit && IsValid(HitTarget) && !HitTarget->IsDead())
	{
		// 대미지 적용
		UGameplayStatics::ApplyDamage(HitTarget, Damage, GetController(), this, UDamageType::StaticClass());
		
		// 스턴과 같은 부가효과도 충돌 시점에 같이 적용하는 로직
		if (CurrentActionSpec.EffectType == ESkillEffectType::Stun)
		{
			HitTarget->ApplyStun(FMath::Max(1, CurrentActionSpec.StunDurationActions));
		}
	}
	
	PendingProjectiles = FMath::Max(0, PendingProjectiles - 1); // 투사체 1개 처리 완료
	TryFinishAction(); // 마지막 투사체면 여기서 FinishAction까지 진행됨
}

void ABattleBaseUnit::NotifyActorOnClicked(FKey ButtonPressed)
{
	Super::NotifyActorOnClicked(ButtonPressed);
	
	// 클릭 시 선택 상태 반전 (또는 선택 로직 실행)
	bIsSelected = true;
	
	UE_LOG(LogTemp, Warning, TEXT("%s Unit Selected!"), *GetName());
	
	// UI 띄우는 이벤트 호출 구현부
}

int32 ABattleBaseUnit::GetSkillNumber()
{
	int32 res = 0;
	
	for (int i = 0; i < Skills.Num(); i++)
	{
		res++;
	}
	
	return res;
}

FSkillRuntimeModifier ABattleBaseUnit::GetSkillRuntimeModifier(const USkillDataAsset* Skill) const
{
	FSkillRuntimeModifier Modifier;
	// TODO: 무기 부품 시스템 여기서 연결
	// Modifier.BonusHitCount += WeaponPartsbouns;
	// Modifier.DamageMulAdd += ...
	// Modifier.DamageMulMul *= ...
	return Modifier;
}

FSkillRuntimeSpec ABattleBaseUnit::BuildRuntimeSpec(const USkillDataAsset* Skill) const
{
	FSkillRuntimeSpec Spec;
	if (!Skill) return Spec;
	
	Spec.TargetType = Skill->TargetType;
	Spec.EffectType = Skill->EffectType;
	
	Spec.DamageMultiplier = Skill->DamageMultiplier;
	Spec.Hitcount = Skill->BaseHitCount;
	
	Spec.HealAmount = Skill->HealAmount;
	Spec.BuffAtk = Skill->BuffAtk;
	Spec.StunDurationActions = Skill->StunDurationActions;
	
	// Delivery(즉발/투사체)
	Spec.DeliveryType = Skill->DeliveryType;
	Spec.ProjectileClass = Skill->ProjectileClass;
	Spec.MuzzleSocketName = Skill->MuzzleSocketName;
	Spec.ProjectileCount = FMath::Max(1, Skill->BaseProjectileCount);
	Spec.bSplitDamageAcrossProjectiles = Skill->bSplitDamageAcrossProjectiles;
	
	// 효과가 Damage가 아니면 타수는 1로 고정 - 나중에 Heal 다중 확장
	if (Spec.EffectType != ESkillEffectType::Damage)
	{
		Spec.Hitcount = 1;
	}
	
	const FSkillRuntimeModifier Modifier = GetSkillRuntimeModifier(Skill);
	Spec.Hitcount = FMath::Max(1, Spec.Hitcount + Modifier.BonusHitCount);
	Spec.ProjectileCount = FMath::Max(1, Spec.ProjectileCount + Modifier.BonusProjectileCount);
	
	Spec.DamageMultiplier = (Spec.DamageMultiplier + Modifier.DamageMulAdd) * Modifier.DamageMulMul;
	Spec.DamageMultiplier = FMath::Max(0.f, Spec.DamageMultiplier);
	
	return Spec;
}

float ABattleBaseUnit::GetEffectiveAttackPower() const
{
	float AddFlat = 0.f;
	float Mul = 1.f;
	
	for (const FActiveAtkBuff& B : ActiveAtkBuffs)
	{
		AddFlat += B.AddFlat;
		Mul *= (1.f + B.AddMultiplier);
	}
	
	return FMath::Max(0.f, (CurrentAttackPower + AddFlat) * Mul);
}

float ABattleBaseUnit::GetMaxHP() const
{
	return UnitData ? FMath::Max(1.f, UnitData->BaseStats.MaxHP) : 100.f;
}

void ABattleBaseUnit::ApplyHealToUnit(ABattleBaseUnit* Target, float HealAmount)
{
	if (!IsValid(Target) || Target->IsDead()) return;
	const float MaxHP = Target->GetMaxHP();
	Target->CurrentHP = FMath::Clamp(Target->CurrentHP + HealAmount, 0.f, MaxHP);

	UE_LOG(LogTemp, Warning, TEXT("[Heal] %s +%.1f => %.1f/%.1f"),
		*Target->GetName(), HealAmount, Target->CurrentHP, MaxHP);
}

// 1hit - 데미지/힐/버프 모두 여기서 처리
void ABattleBaseUnit::ApplyOneHit()
{
	if (!CurrentSkillData) return;
	
	const FSkillRuntimeSpec& Spec = CurrentActionSpec; // 캐싱된 스펙 사용하여 일관성 보장
	
	// 이미 타수 다 쳤으면 무시
	if (Spec.EffectType == ESkillEffectType::Damage)
	{
		if (ActionHitsApplied >= ActionTotalHits) return;
	}
	
	// 유효 타겟만 정리
	TArray<ABattleBaseUnit*> Targets;
	for (auto& W : CurrentActionTargets)
	{
		if (ABattleBaseUnit* T = W.Get())
		{
			if (!T->IsDead())
			{
				Targets.Add(T);
			}
		}
	}
	if (Targets.Num() == 0) return;

	switch (Spec.EffectType)
	{
	case ESkillEffectType::Damage:
		{
			const float BaseATK = GetEffectiveAttackPower();
			const float Damage = FMath::Max(1.f, BaseATK * Spec.DamageMultiplier);
				
			for (ABattleBaseUnit* T : Targets)
			{
				UGameplayStatics::ApplyDamage(T, Damage, GetController(), this, UDamageType::StaticClass());
				UE_LOG(LogTemp, Warning, TEXT("[SkillDamage] %s -> %s : %.1f (hit %d/%d)"),
			*GetName(), *T->GetName(), Damage, ActionHitsApplied + 1, ActionTotalHits);
			}
					
			ActionHitsApplied++;
			break;
		}
		
	case ESkillEffectType::Heal:
		{
			for (ABattleBaseUnit* T : Targets)
			{
				ApplyHealToUnit(T, Spec.HealAmount);
			}
			// Heal은 1회로 끝
			ActionHitsApplied = ActionTotalHits;
			break;
		}
		
	case ESkillEffectType::BuffATK:
		{
			for (ABattleBaseUnit* T : Targets)
			{
				FActiveAtkBuff Buff;
				Buff.AddMultiplier = Spec.BuffAtk.AddMultiplier;
				Buff.AddFlat = Spec.BuffAtk.AddFlat;
				Buff.RemainingActions = FMath::Max(1, Spec.BuffAtk.DurationActions);
				
				T->ActiveAtkBuffs.Add(Buff);
				
				UE_LOG(LogTemp, Warning, TEXT("[BuffATK] %s buffed: +mul=%.2f +flat=%.1f dur=%d"),
				*T->GetName(), Buff.AddMultiplier, Buff.AddFlat, Buff.RemainingActions);
				ActionHitsApplied = ActionTotalHits;
				break;
			}
		}
		
	case ESkillEffectType::Stun:
		{
			for (ABattleBaseUnit* Unit : Targets)
			{
				Unit->ApplyStun(FMath::Max(1, Spec.StunDurationActions));
			}
			ActionHitsApplied = ActionTotalHits;
			break;
		}
	}
}

void ABattleBaseUnit::ApplyRemainingHits()
{
	while (ActionHitsApplied < ActionTotalHits)
	{
		ApplyOneHit();
	}
}

void ABattleBaseUnit::TickBuffDuration_OnActionEnd()
{
	// 버프 지속 시간 처리 - 감소
	for (int32 i = ActiveAtkBuffs.Num() - 1; i >= 0; i--)
	{
		ActiveAtkBuffs[i].RemainingActions--;
		if (ActiveAtkBuffs[i].RemainingActions <= 0)
		{
			ActiveAtkBuffs.RemoveAt(i);
		}
	}
	
	// 스턴 지속 시간 처리 - 본인 턴 종료 기준으로 1 감소
	if (StunRemainingActions > 0)
	{
		StunRemainingActions--;
	}
}

void ABattleBaseUnit::SpawnProjectileVolley(const TArray<ABattleBaseUnit*>& Targets, float WaveDamage)
{
	const FSkillRuntimeSpec& Spec = CurrentActionSpec;
	
	if (!Spec.ProjectileClass)
	{
		// 투사체 클래스 없으면 안전하게 즉발로 폴백
		for (ABattleBaseUnit* T : Targets)
		{
			UGameplayStatics::ApplyDamage(T, WaveDamage, GetController(), this, UDamageType::StaticClass());
		}
		return;
	}
	
	UWorld* World = GetWorld();
	if (!World) return;
	
	const int32 ProjCount = FMath::Max(1, Spec.ProjectileCount);
	const float DamagePerProjectile = Spec.bSplitDamageAcrossProjectiles
		? (WaveDamage / (float)ProjCount) // 투사체 수가 늘어도 총합 유지
		: WaveDamage;					// 투사체 수가 늘면 총합 증가
	
	// 발사 위치
	FVector MuzzleLoc = GetActorLocation();
	if (GetMesh() && Spec.MuzzleSocketName != NAME_None && GetMesh()->DoesSocketExist(Spec.MuzzleSocketName))
	{
		MuzzleLoc = GetMesh()->GetSocketLocation(Spec.MuzzleSocketName);
	}
	
	for (ABattleBaseUnit* Target : Targets)
	{
		if (!IsValid(Target) || Target->IsDead()) continue;
		
		for (int32 i = 0; i < ProjCount; ++i)
		{
			const FRotator SpawnRot = (Target->GetActorLocation() - MuzzleLoc).Rotation();
			
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = this; // 발사자 표시
			//SpawnParams.Instigator = GetController() ? GetPawn() : nullptr;
			SpawnParams.Instigator = this;
			
			ABattleProjectile* P = World->SpawnActor<ABattleProjectile>(Spec.ProjectileClass, MuzzleLoc, SpawnRot, SpawnParams);
			if (!P) continue;
			
			PendingProjectiles++; // 투사체 1개 처리 대기 증가
			P->InitProjectile(this, Target, DamagePerProjectile); // 투사체에 발사/타겟/대미지 전달
		}
	}
}
