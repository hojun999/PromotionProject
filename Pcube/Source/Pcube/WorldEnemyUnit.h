// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LootCorpseActor.h"
#include "WorldBaseUnit.h"
#include  "AIController.h"
#include "WorldEnemyUnit.generated.h"

class USpawnDataAsset;
class UBoxComponent;
class USphereComponent;

UENUM(BlueprintType)
enum class EEnemyAIState : uint8
{
	Patrol  UMETA(DisplayName="Patrol"),  // 순찰 중
	Chase   UMETA(DisplayName="Chase"),   // 추격 중
	Idle    UMETA(DisplayName="Idle"),    // 대기 (순찰 포인트 없을 때)
};

UCLASS()
class PCUBE_API AWorldEnemyUnit : public AWorldBaseUnit
{
	GENERATED_BODY()

public:
	AWorldEnemyUnit();
	
	void InitializeEncounterInfo(FName InEncounterID, UUnitDataAsset* InUnitData, USpawnDataAsset* InSpawnData, bool bAlreadyDefeated);
	
	// EncounterSpawner에서 스폰 직후 호출
	void SetPatrolPoints(const TArray<FVector>& InPatrolPoints) { PatrolPoints = InPatrolPoints; }
	void SetBossEncounter(bool bInIsBoss) { bIsBossEncounter = bInIsBoss; }
	
	// 게임 정지/재개 시 AI 이동 제어
	void PausePatrol();
	void ResumePatrol();
	
protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void InitFromUnitData(UUnitDataAsset* InUnitData) override;
	
	// ── 감지 이벤트 ────────────────────────────────────────
	UFUNCTION()
	void OnDetectOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
						UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
						bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnDetectEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
						UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

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
	void ConvertToCorpse();
	
	void Loot();
	
	// ── 순찰 / 추격 ────────────────────────────────────────

	// 순찰 경유 포인트 (월드 좌표). 에디터에서 직접 입력
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI|Patrol")
	TArray<FVector> PatrolPoints;

	// 순찰 이동 속도
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI|Patrol", meta=(ClampMin="0.0"))
	float PatrolSpeed = 180.f;

	// 추격 이동 속도
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI|Chase", meta=(ClampMin="0.0"))
	float ChaseSpeed = 450.f;

	// 순찰 포인트 도착 판정 허용 오차 (cm)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI|Patrol", meta=(ClampMin="10.0"))
	float PatrolAcceptanceRadius = 80.f;

	// ── 컴포넌트 ───────────────────────────────────────────
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Component")
	TObjectPtr<UStaticMeshComponent> DeadStaticMeshComp = nullptr;

	UPROPERTY(VisibleAnywhere, Category="AI")
	USphereComponent* DetectSphere;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Battle")
	class USpawnDataAsset* SpawnData;

	UPROPERTY(EditAnywhere, Category="Visual")
	UStaticMesh* DeadMesh = nullptr;

	UPROPERTY(VisibleAnywhere)
	UBoxComponent* InteractBox = nullptr;

	UPROPERTY(BlueprintReadOnly)
	bool bIsCorpse = false;

private:
	// ── AI 상태 ────────────────────────────────────────────
	UPROPERTY(VisibleAnywhere, Category="AI|Debug")
	EEnemyAIState AIState = EEnemyAIState::Idle;

	// 현재 향하는 순찰 포인트 인덱스
	int32 CurrentPatrolIndex = 0;

	// 순찰 방향 (+1: 앞으로, -1: 뒤로)
	int32 PatrolDirection = 1;
	
	// 감지된 플레이어
	UPROPERTY()
	TWeakObjectPtr<AActor> ChaseTarget = nullptr;

	void UpdatePatrol(float DeltaTime);
	void UpdateChase(float DeltaTime);
	void SetAIState(EEnemyAIState NewState);
	void MoveToNextPatrolPoint(); // 네비메시 기반 순찰 이동
	
	
	UFUNCTION()
	void OnPatrolMoveCompleted(FAIRequestID RequestID, EPathFollowingResult::Type Result);

	bool bWaitingForPatrolMove = false; // 이동 중 중복 명령 방지
	bool bPatrolPaused = false; // Pause 상태
	
	UPROPERTY(Transient)
	bool bEncounterLocked = false;

	bool bIsBossEncounter = false;
	
	// ── 루팅 ───────────────────────────────────────────────
protected:
	UPROPERTY(EditDefaultsOnly, Category="Loot")
	TSubclassOf<ALootCorpseActor> LootCorpseClass;

	UPROPERTY(EditAnywhere, Category="Encounter")
	FName EncounterID = NAME_None;

	bool TrySpawnCorpseIfDefeated();
};
