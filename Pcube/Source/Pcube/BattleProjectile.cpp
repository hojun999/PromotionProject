// Fill out your copyright notice in the Description page of Project Settings.


#include "BattleProjectile.h"
#include "BattleBaseUnit.h"

// Sets default values
ABattleProjectile::ABattleProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComp"));
	SetRootComponent(CollisionComp);
	CollisionComp->InitSphereRadius(10.f);
	CollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	CollisionComp->OnComponentBeginOverlap.AddDynamic(this, &ABattleProjectile::OnOverlap);
	
	MoveComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("MoveComp"));
	MoveComp->InitialSpeed = 0.f; // BeginPlay에서 즉시 이동 방지, InitProjectile에서 활성화
	MoveComp->MaxSpeed = 2000.f;
	MoveComp->bRotationFollowsVelocity = true;
	MoveComp->bShouldBounce = false;
	MoveComp->SetAutoActivate(false); // InitProjectile 전까지 이동 비활성화
	
	InitialLifeSpan = 3.0f; // 너무 오래 남는 문제 방지
}

// Called when the game starts or when spawned
void ABattleProjectile::BeginPlay()
{
	Super::BeginPlay();
}

void ABattleProjectile::InitProjectile(ABattleBaseUnit* InSource, ABattleBaseUnit* InTarget, float InDamage)
{
	SourceUnit = InSource;
	TargetUnit = InTarget;
	Damage = InDamage;

	if (TargetUnit)
	{
		TargetUnit->OnUnitDied.RemoveDynamic(this, &ABattleProjectile::HandleTargetUnitDied);
		TargetUnit->OnUnitDied.AddDynamic(this, &ABattleProjectile::HandleTargetUnitDied);
	}
	
	// 호밍: 타겟이 있으면 가속으로 따라가게
	if (MoveComp && TargetUnit)
	{
		MoveComp->bIsHomingProjectile = true;
		MoveComp->HomingTargetComponent = TargetUnit->GetRootComponent();
		MoveComp->HomingAccelerationMagnitude = 8000.f;
	}
	
	// 타겟/호밍 설정 완료 후 이동 활성화
	if (MoveComp)
	{
		MoveComp->InitialSpeed = 2000.f;
		MoveComp->Velocity = GetActorForwardVector() * MoveComp->InitialSpeed;
		MoveComp->Activate(true); // 이제 이동 시작
	}
}

void ABattleProjectile::OnOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (bResolved) return;
	
	ABattleBaseUnit* HitUnit = Cast<ABattleBaseUnit>(OtherActor);
	if (!IsValid(HitUnit)) return;
	
	// 목표 지정이 있으면 해당 목표만 맞게
	if (TargetUnit && HitUnit != TargetUnit) return;
	
	bResolved = true;
	
	if (SourceUnit)
	{
		SourceUnit->NotifyProjectileResolved(HitUnit, Damage, true); // 충돌 성공
	}
	
	Destroy();
}

void ABattleProjectile::HandleTargetUnitDied()
{
	if (!bResolved)
	{
		Destroy();
	}
}

void ABattleProjectile::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (TargetUnit)
	{
		TargetUnit->OnUnitDied.RemoveDynamic(this, &ABattleProjectile::HandleTargetUnitDied);
	}

	Super::EndPlay(EndPlayReason);
	
	// 수명 종료 / 강제 제거 등으로 충돌 없이 사라지는 경우도 처리 완료로 처리
	if (!bResolved)
	{
		bResolved = true;
		if (SourceUnit)
		{
			SourceUnit->NotifyProjectileResolved(TargetUnit, Damage, false); // 충돌 실패(미스/소멸)
		}
	}
}
