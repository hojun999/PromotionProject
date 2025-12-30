// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BaseUnit.generated.h"

class USkillDataAsset;

UCLASS()
class PCUBE_API ABaseUnit : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ABaseUnit();

	// 스탯
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float MaxHP;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	float CurrentHP;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float AttackPower;
	
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float Speed;
	
	UPROPERTY(BlueprintReadOnly)
	float CurrentActionValue;
	
	// 스킬 목록 (아군/적 개수 제한 없이 데이터로 관리)
	UPROPERTY(EditAnywhere, Category = "Skills")
	TArray<USkillDataAsset*> Skills;
	
	// 언리얼 기본 데미지 함수
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	
	// 공통 일반 공격 함수
	virtual  void BasicAttack(ABaseUnit* Target);
	
	// 사망 처리 함수
	virtual  void Die();
	
	// 턴 시작 시 호출되는 함수
	virtual  void OnTurnStarted();
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// 마우스 클릭 시 호출되는 엔진 기본 이벤트
	virtual void NotifyActorOnClicked(FKey ButtonPressed = EKeys::LeftMouseButton) override;
	
	// 선택 상태 관리 변수
	UPROPERTY(BlueprintReadOnly, Category="Selection")
	bool bIsSelected;
	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
