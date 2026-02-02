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
