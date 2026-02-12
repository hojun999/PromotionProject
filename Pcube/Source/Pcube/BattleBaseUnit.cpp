// Fill out your copyright notice in the Description page of Project Settings.


#include "BattleBaseUnit.h"
#include "SkillDataAsset.h"
#include "UnitDataAsset.h"
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
	// 사망 애니메이션, 객체 제거 및 시체로 변경 로직
	UE_LOG(LogTemp, Error, TEXT("%s has died."), *GetName());
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
