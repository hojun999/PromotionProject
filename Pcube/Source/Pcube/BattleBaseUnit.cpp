// Fill out your copyright notice in the Description page of Project Settings.


#include "BattleBaseUnit.h"
#include "SkillDataAsset.h"
#include "UnitDataAsset.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ABattleBaseUnit::ABattleBaseUnit()
{
	// tick 사용 x
	PrimaryActorTick.bCanEverTick = false;
}

// Called when the game starts or when spawned
void ABattleBaseUnit::BeginPlay()
{
	Super::BeginPlay();
	
	// 시작할 때 데이터 에셋으로부터 값 가져옴
	if (UnitData)
	{
		CurrentHP = UnitData->BaseStats.MaxHP;
		CurrentSpeed = UnitData->BaseStats.Speed;
		CurrentAttackPower = UnitData->BaseStats.AttackPower;
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
