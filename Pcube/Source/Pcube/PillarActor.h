#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PillarActor.generated.h"

UCLASS()
class PCUBE_API APillarActor : public AActor
{
	GENERATED_BODY()
    
public:    
	APillarActor();

protected:
	virtual void BeginPlay() override;

public:    
	virtual void Tick(float DeltaTime) override;

private:
	// 초기 위치 저장
	FVector InitialLocation;

	// 적용될 랜덤 수치들
	float RandomAmplitude;    // 이동 범위
	float RandomFrequency;    // 이동 속도
	float RandomRotationSpeed; // 회전 속도
	float TimeOffset;         // 시작 시점 차이 (모든 기둥이 똑같이 움직이지 않도록 함)

	UPROPERTY(EditAnywhere, Category = "Pillar Animation")
	float MaxAmplitude = 100.f;

	UPROPERTY(EditAnywhere, Category = "Pillar Animation")
	float MaxRotationSpeed = 90.f;
};