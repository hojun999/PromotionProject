// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BattleBaseUnit.generated.h"

class UUnitDataAsset;
class USkillDataAsset;

UCLASS()
class PCUBE_API ABattleBaseUnit : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ABattleBaseUnit();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	UUnitDataAsset* UnitData;
	
	// 스탯
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	float CurrentHP;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	float CurrentSpeed;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	float CurrentAttackPower;
	
	virtual void InitUnit(UUnitDataAsset* TransferredUnitData);
	
	// 턴 관리를 위한 행동 수치
	UPROPERTY(BlueprintReadOnly)
	float CurrentActionValue;
	
	// 스킬 목록 (아군/적 개수 제한 없이 데이터로 관리)
	UPROPERTY(EditAnywhere, Category = "Skills")
	TArray<USkillDataAsset*> Skills;
	
	// --- 전투 핵심 로직
	// 언리얼 기본 데미지 함수
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	
	// 공통 일반 공격 함수
	virtual  void BasicAttack(ABattleBaseUnit* Target);
	
	// 사망 처리 함수
	virtual  void Die();
	
	// 턴 매니저가 호출할 함수
	// TODO: 특정 스킬 사용 후, 몇 턴 동안 매 턴 시작마다 발동하는 스킬 추가
	virtual void OnTurnStarted();
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Visual")
	UStaticMeshComponent* StaticMeshComp;
	
	// 마우스 클릭 시 호출되는 엔진 기본 이벤트
	virtual void NotifyActorOnClicked(FKey ButtonPressed = EKeys::LeftMouseButton) override;
	
	// 선택 상태 관리 변수
	UPROPERTY(BlueprintReadOnly, Category="Selection")
	bool bIsSelected;
	
	

};
