// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "BattleProjectile.generated.h"

class ABattleBaseUnit;

UCLASS()
class PCUBE_API ABattleProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	ABattleProjectile();

	void InitProjectile(ABattleBaseUnit* InSource, ABattleBaseUnit* InTarget, float InDamage); // 투사체 초기화 - 발사자/타겟/데미지
	
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	UFUNCTION()
	void OnOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
				   UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
				   bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void HandleTargetUnitDied();
	
private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USphereComponent> CollisionComp; // 충돌 판정용
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UProjectileMovementComponent> MoveComp; // 이동 처리용
	
	UPROPERTY()
	TObjectPtr<ABattleBaseUnit> SourceUnit; // 발사자
	
	UPROPERTY()
	TObjectPtr<ABattleBaseUnit> TargetUnit; // 타겟 (호밍/판정)
	
	UPROPERTY()
	float Damage = 0.f; // 투사체 1발당 데미지
	
	UPROPERTY()
	bool bResolved = false; // 중복 방지
};
