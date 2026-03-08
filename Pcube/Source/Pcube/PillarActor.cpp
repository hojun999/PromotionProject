#include "PillarActor.h"

APillarActor::APillarActor()
{
	PrimaryActorTick.bCanEverTick = true;
}

void APillarActor::BeginPlay()
{
	Super::BeginPlay();

	// 1. 초기 위치 저장
	InitialLocation = GetActorLocation();

	// 2. 플레이마다 임의의 수치 할당 (랜덤성 부여)
	RandomAmplitude = FMath::RandRange(20.f, MaxAmplitude);
	RandomFrequency = FMath::RandRange(0.5f, 2.0f);
    
	// Z축 +, - 방향 중 하나로 랜덤 회전 속도 결정
	float Dir = FMath::RandBool() ? 1.f : -1.f;
	RandomRotationSpeed = FMath::RandRange(30.f, MaxRotationSpeed) * Dir;

	// 모든 기둥이 동시에 위아래로 움직이지 않도록 시작 오프셋 설정
	TimeOffset = FMath::RandRange(0.f, 10.f);
}

void APillarActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	float CurrentTime = GetWorld()->GetTimeSeconds() + TimeOffset;

	// 1. 위아래로 왔다갔다함 (Sine 파형 활용)
	FVector NewLocation = InitialLocation;
	NewLocation.Z += FMath::Sin(CurrentTime * RandomFrequency) * RandomAmplitude;
	SetActorLocation(NewLocation);

	// 2. Z축 방향으로 회전
	FRotator NewRotation = GetActorRotation();
	NewRotation.Yaw += RandomRotationSpeed * DeltaTime;
	SetActorRotation(NewRotation);
}