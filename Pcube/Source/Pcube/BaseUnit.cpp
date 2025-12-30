// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseUnit.h"
#include "SkillDataAsset.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ABaseUnit::ABaseUnit()
{
	MaxHP = 100.f;
	AttackPower = 20.f;
	
}

// Called when the game starts or when spawned
void ABaseUnit::BeginPlay()
{
	Super::BeginPlay();
	CurrentHP = MaxHP;
}

// Called every frame
void ABaseUnit::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ABaseUnit::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void ABaseUnit::BasicAttack(ABaseUnit* Target)
{
	if (Target && AttackPower > 0)
	{
		// 언리얼 표준 데미지 전달 함수
		UGameplayStatics::ApplyDamage(Target, AttackPower, GetController(), this, UDamageType::StaticClass());
	}
}

float ABaseUnit::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	
	CurrentHP = FMath::Clamp(CurrentHP - ActualDamage,  0.f,  MaxHP);
	UE_LOG(LogTemp, Warning, TEXT("%s took %f damage. Remaining HP: %f"), *GetName(), ActualDamage, CurrentHP);
	
	if (CurrentHP <= 0.f)
	{
		Die();
	}

	return ActualDamage;
}

void ABaseUnit::Die()
{
	// 사망 애니메이션, 객체 제거 및 시체로 변경 로직
	UE_LOG(LogTemp, Error, TEXT("%s has died."), *GetName());
}

void ABaseUnit::OnTurnStarted()
{
	
}

void ABaseUnit::NotifyActorOnClicked(FKey ButtonPressed)
{
	Super::NotifyActorOnClicked(ButtonPressed);
	
	// 클릭 시 선택 상태 반전 (또는 선택 로직 실행)
	bIsSelected = true;
	
	UE_LOG(LogTemp, Warning, TEXT("%s Unit Selected!"), *GetName());
	
	// UI 띄우는 이벤트 호출 구현부
}
