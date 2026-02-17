// Fill out your copyright notice in the Description page of Project Settings.


#include "BattleBaseUnit.h"
#include "SkillDataAsset.h"
#include "UnitDataAsset.h"
#include "Chaos/Deformable/MuscleActivationConstraints.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ABattleBaseUnit::ABattleBaseUnit()
{
	StaticMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComp"));
	StaticMeshComp->SetupAttachment(RootComponent);
	
	// ACharacter를 상속받으므로 깁본 skeletalmesh 숨기기
	// TODO: 이후에 Skeletalmesh 사용할 때 아래 내용 삭제
	if (GetMesh())
	{
		GetMesh()->SetHiddenInGame(true);
	}
	
	CineCameraArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("CineCameraArmComp"));
	CineCameraArmComp->SetupAttachment(RootComponent);
	CineCameraArmComp->TargetArmLength = 800.0f;
	CineCameraArmComp->SetRelativeRotation(FRotator(-30.0f, 45.0f, 0.0f));
	CineCameraArmComp->bDoCollisionTest = false;
	CineCameraArmComp->bUsePawnControlRotation = false;
	
	CineCameraComp = CreateDefaultSubobject<UCineCameraComponent>(TEXT("CineCameraComp"));
	CineCameraComp->SetupAttachment(CineCameraArmComp, USpringArmComponent::SocketName);
	CineCameraComp->CurrentFocalLength = 50.0f;
	
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
	if (!TransferredUnitData)
	{
		return;
	}
	
	if (TransferredUnitData)
	{
		UnitData = TransferredUnitData;
		
		CurrentHP = UnitData->BaseStats.MaxHP;
		CurrentSpeed = UnitData->BaseStats.Speed;
		CurrentAttackPower = UnitData->BaseStats.AttackPower;
		
		UE_LOG(LogTemp, Log, TEXT("%s 유닛 데이터 전달 완료!"), *UnitData->UnitName);
	}
	
	// 메시 설정 (Character 상속 시 GetMesh(), 일반 Actor 상속 시 컴포넌트 찾아야 됨)
	if (StaticMeshComp && TransferredUnitData->UnitStaticMesh)
	{
		StaticMeshComp->SetStaticMesh((TransferredUnitData->UnitStaticMesh));
		
		// 여기서 스케일이나 머티리얼 추가 조정 가능
	}
}

void ABattleBaseUnit::FinishAction()
{
	UE_LOG(LogTemp, Log, TEXT("%s 유닛 행동 종료!"), *GetName());
	
	// 신호 발송 - 구독하고 있는 모든 곳에 알림
	if (OnActionFinished.IsBound())
	{
		OnActionFinished.Broadcast();
	}
}

void ABattleBaseUnit::BasicAttack(ABattleBaseUnit* Target)
{
	if (Target && CurrentAttackPower > 0)
	{
		// 언리얼 표준 데미지 전달 함수
		UGameplayStatics::ApplyDamage(Target, CurrentAttackPower, GetController(), this, UDamageType::StaticClass());
	}
}

void ABattleBaseUnit::ExecuteAction(USkillDataAsset* SkillData, AActor* Target)
{
	if (!SkillData || !Target) return;
	
	CurrentSkillData = SkillData;
	CurrentActionTarget = Target;
	
	// 애니메이션 몽타주 재생
	UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
	
	// 몽타주 / Anim 인스턴스 없으면 최소 구현으로 즉시 종료 - 턴 진행 방지
	if (!AnimInstance || !SkillData->ActionMontage)
	{
		UE_LOG(LogTemp, Warning, TEXT("ExecuteAction: No AnimInstance or Montage. Finishing action immediately."));
		FinishAction();
		return;
	}
	
	// 몽타주 기반 실행
	AnimInstance->OnPlayMontageNotifyBegin.RemoveDynamic(this, &ABattleBaseUnit::HandleHitNotify);	// 기존 바인딩 해제
	AnimInstance->OnPlayMontageNotifyBegin.AddDynamic(this, &ABattleBaseUnit::HandleHitNotify);		// 바인딩 할당
	
	const float PlayResult = AnimInstance->Montage_Play(SkillData->ActionMontage);
	if (PlayResult <= 0.f)
	{
		UE_LOG(LogTemp, Warning, TEXT("ExecuteAction: Montage_Play failed. Finishing action immediately."));
		FinishAction();
		return;
	}
	
	// 몽타주 종료 시점에 FinishAction() 호출 연결
	FOnMontageEnded EndDelegate;
	EndDelegate.BindUObject(this, &ABattleBaseUnit::OnActionMontageEnded);
	AnimInstance->Montage_SetEndDelegate(EndDelegate, SkillData->ActionMontage);
}

void ABattleBaseUnit::HandleHitNotify(FName NotifyName, const FBranchingPointNotifyPayload& Payload)
{
	// 몽타주에서 설정한 노티파이 이름이 "Hit"인 경우에만 로직 실행
	if (NotifyName == FName("Hit"))
	{
		UE_LOG(LogTemp, Log, TEXT("타격 발생! 데미지를 입힙니다."));
		ApplyCurrentSkillDamage();
	}
}

void ABattleBaseUnit::ApplyCurrentSkillDamage()
{
	if (!CurrentSkillData || !CurrentActionTarget) return;
	
	ABattleBaseUnit* TargetUnit = Cast<ABattleBaseUnit>(CurrentActionTarget);
	if (!TargetUnit || TargetUnit->IsDead()) return;
	
	// 단순 데미지 처리 - 로그 확인용
	const float Damage = FMath::Max(1.f, CurrentAttackPower);
	
	UE_LOG(LogTemp, Warning, TEXT("[Damage] %s -> %s : %0.1f"),
		*GetName(), *TargetUnit->GetName(), Damage);
	
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Yellow,
			FString::Printf(TEXT("[Damage] %s -> %s : %0.1f"), *GetName(), *TargetUnit->GetName(), Damage));
	}
	
	UGameplayStatics::ApplyDamage(TargetUnit, Damage, GetController(), this, UDamageType::StaticClass());
}

void ABattleBaseUnit::OnActionMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	UE_LOG(LogTemp, Log, TEXT("%s action montage ended (Interrupted=%d)"), *GetName(), bInterrupted ? 1 : 0);
	
	// Notify 바인딩 정리 - 중복 방지
	if (UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr)
	{
		AnimInstance->OnPlayMontageNotifyBegin.RemoveDynamic(this, &ABattleBaseUnit::HandleHitNotify);
	}
	
	FinishAction();
}

float ABattleBaseUnit::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	
	float MaxHP = UnitData ? UnitData->BaseStats.MaxHP : ActualDamage;
	CurrentHP = FMath::Clamp(CurrentHP - ActualDamage,  0.f,  MaxHP);
	
	UE_LOG(LogTemp, Warning, TEXT("%s took %f damage. Remaining HP: %f/%f"), *GetName(), ActualDamage, CurrentHP, MaxHP);
	
	if (CurrentHP <= 0.f)
	{
		Die();
	}

	return ActualDamage;
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
}

void ABattleBaseUnit::NotifyActorOnClicked(FKey ButtonPressed)
{
	Super::NotifyActorOnClicked(ButtonPressed);
	
	// 클릭 시 선택 상태 반전 (또는 선택 로직 실행)
	bIsSelected = true;
	
	UE_LOG(LogTemp, Warning, TEXT("%s Unit Selected!"), *GetName());
	
	// UI 띄우는 이벤트 호출 구현부
}

void ABattleBaseUnit::OnTurnStarted()
{
	
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
