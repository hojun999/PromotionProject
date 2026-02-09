// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SpawnDataAsset.h"
#include "Subsystems/WorldSubsystem.h"
#include "BattleTurnManager.h"
#include "BattleControlSubsystem.generated.h"


class ABattleBaseUnit;




UENUM(BlueprintType)
enum class EBattleState : uint8
{
	Ready,			// 유닛 스폰 및 초기화
	NewRound,		// 매 라운드 시작 (주사위 굴리기)
	WaitTurn,		// 다음 유닛 결정
	ActionInput,	// 플레이어 입력 대기 (또는 AI 행동 결정)
	ActionExecute,	// 애니메이션 / 스킬 사용 연출 중
	CheckCondition,	// 승패 판정
	Finished		// 전투 종료
};

// 상태 변경 알림 - UI 모드 전환용
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBattleStateChanged, EBattleState, NewState);
// 현재 턴 유닛 정보 전달 - 스킬 UI 갱신용
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTurnUnitChanged, ABattleBaseUnit*, ActiveUnit);
// 유닛 포인터 배열을 전달하는 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTurnOrderUpdated, const TArray<AActor*>&, ActionOrder);

UCLASS()
class PCUBE_API UBattleControlSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
	
	
public:
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	
	UPROPERTY(BlueprintAssignable, Category="Battle|UI")
	FOnBattleStateChanged OnBattleStateChanged;
	
	UPROPERTY(BlueprintAssignable, Category="Battle|UI")
	FOnTurnUnitChanged OnTurnUnitChanged;
	
	UPROPERTY(BlueprintAssignable, Category="Battle|UI")
	FOnTurnOrderUpdated OnTurnOrderChanged;
	
	// 유닛들을 스폰하는 함수
	void SpawnBattleUnits();
	
	void SetState(EBattleState NewState);
	void HandleNewRound();
	void HandleWaitTurn();
	void HandleActionInput();
	void HandleCheckCondition();
	
	UFUNCTION()
	void OnUnitActionComplete();

	
protected:
	void OnBattleSetupFinished();
	
	void HandleEnemyAI(ABattleBaseUnit* EnemyUnit);
	
	EBattleState CurrentState;
	
	UPROPERTY()
	UBattleTurnManager* TurnManager;
	
	AActor* CurrentActionUnit;
	
	// 현재 전투에 참여 중인 적 유닛들을 담는 배열 (턴 관리용)
	UPROPERTY()
	TArray<AActor*> SpawnedEnemies;
	
	// 현재 전투에 참여 중인 플레이어 유닛들을 담는 배열
	UPROPERTY()
	TArray<AActor*> SpawnedAllies;
	
private:
	// 유닛 스폰의 공통적인 부분을 담당하는 함수
	ABattleBaseUnit* SpawningLogic(const FUnitSpawnInfo& UnitInfo);
	
	int32 GetAliveUnitCount(const TArray<AActor*>& UnitList);
};
