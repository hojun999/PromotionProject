// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WorldBaseUnit.h"
#include "WorldEnemyUnit.generated.h"

class USpawnDataAsset;
class UBoxComponent;
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
	
	void InitializeEncounterInfo(FName InEncounterID, UUnitDataAsset* InUnitData, USpawnDataAsset* InSpawnData, bool bAlreadyDefeated);
	
protected:
	virtual void BeginPlay() override;
	
	virtual void InitFromUnitData(UUnitDataAsset* InUnitData) override;
	
	// 감지 이벤트 함수
	UFUNCTION()
	void OnDetectOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
						UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
						bool bFromSweep, const FHitResult& SweepResult);
	
	UFUNCTION()
	void OnEncounterOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
						UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
						bool bFromSweep, const FHitResult& SweepResult);
	
	UFUNCTION()
	void OnCorpseBeginOverlap(UPrimitiveComponent* Overlapped, AActor* OtherActor,
							  UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
							  bool bFromSweep, const FHitResult& Sweep);

	UFUNCTION()
	void OnCorpseEndOverlap(UPrimitiveComponent* Overlapped, AActor* OtherActor,
							UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
	
	void StartEncounter(AActor* PlayerActor);
	
	UFUNCTION(BlueprintCallable)
	void ConvertToCorpse(); // BeginPlay에서 변환 트리거
	
	void Loot(); // 프로토타입
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Component")
	TObjectPtr<UStaticMeshComponent> DeadStaticMeshComp = nullptr;
	
	// 플레이어 감지용 콜라이더
	UPROPERTY(VisibleAnywhere, Category="AI")
	USphereComponent* DetectSphere;
	
	// 해당 정보는 에디터 인스펙터에서 어떤 적 그룹을 전투에 참여시킬지 선택
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Battle")
	class USpawnDataAsset* SpawnData;
	
	UPROPERTY(EditAnywhere, Category="Visual")
	UStaticMesh* DeadMesh = nullptr;
	
	UPROPERTY(VisibleAnywhere)
	UBoxComponent* InteractBox = nullptr;
	
	UPROPERTY(EditAnywhere, Category="Encounter")
	FName EncounterID = NAME_None;
	
	UPROPERTY(BlueprintReadOnly)
	bool bIsCorpse = false;
	
	UPROPERTY()
	bool bEncounterStarted = false;
};
