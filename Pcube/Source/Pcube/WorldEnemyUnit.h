// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WorldBaseUnit.h"
#include "WorldEnemyUnit.generated.h"

class USphereComponent;

/**
 * 
 */
UCLASS()
class PCUBE_API AWorldEnemyUnit : public AWorldBaseUnit
{
	GENERATED_BODY()

public:
	AWorldEnemyUnit();
	
protected:
	virtual void BeginPlay() override;
	
	// 해당 정보는 에디터 인스펙터에서 어떤 적 그룹을 전투에 참여시킬지 선택
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Battle")
	class UEncounterDataAsset* EncounterData;
	
	void StartEncounter(AActor* PlayerActor);
	
	// 플레이어 감지용 콜라이더
	UPROPERTY(VisibleAnywhere, Category="AI")
	USphereComponent* DetectSphere;
	
	// 감지 이벤트 함수
	UFUNCTION()
	void OnDetectOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
						UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
						bool bFromSweep, const FHitResult& SweepResult);
	
	UFUNCTION()
	void OnEncounterOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
						UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
						bool bFromSweep, const FHitResult& SweepResult);
};
