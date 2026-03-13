// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SpawnDataAsset.h"
#include "Subsystems/WorldSubsystem.h"
#include "BattleTurnManager.h"
#include "SkillDataAsset.h"
#include "BattleControlSubsystem.generated.h"


class ABattleBaseUnit;

UENUM(BlueprintType)
enum class EBattleState : uint8
{
	Ready,			// 유닛 스폰 및 초기화
	NewRound,		// 매 라운드 시작 (주사위 굴리기)
	WaitTurn,		// 다음 유닛 결정
	ActionInput,	// 플레이어 입력 대기 (또는 AI 행동 결정)
	TargetSelection, // 공격할 적 유닛 선택
	ActionExecute,	// 애니메이션 / 스킬 사용 연출 중
	CheckCondition,	// 승패 판정
	Finished		// 전투 종료
};

UENUM(BlueprintType)
enum class EBattleResult : uint8
{
	None,
	Victory,
	Defeat
};

// 상태 변경 알림 - UI 모드 전환용
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBattleStateChanged, EBattleState, NewState);
// 현재 턴 유닛 정보 전달 - 스킬 UI 갱신용
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTurnUnitChanged, ABattleBaseUnit*, ActiveUnit);
// 유닛 포인터 배열을 전달하는 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTurnOrderUpdated, const TArray<AActor*>&, ActionOrder);
// 적 유닛 타겟팅 완료 시 Broadcast할 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTargetChanged, AActor* , NewTarget);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBattleFinished, EBattleResult, Result);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBattleUnitsSpawned, const TArray<AActor*>&, Allies, const TArray<AActor*>&, Enemis);

UCLASS()
class PCUBE_API UBattleControlSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
	
	
public:
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	
	UPROPERTY(BlueprintAssignable, Category="Battle|UI")
	FOnBattleStateChanged OnBattleStateChanged;
	
	UPROPERTY(BlueprintAssignable, Category="Battle|UI")
	FOnBattleUnitsSpawned OnBattleUnitsSpawned;
	
	UPROPERTY(BlueprintAssignable, Category="Battle|UI")
	FOnTurnUnitChanged OnTurnUnitChanged;
	
	UPROPERTY(BlueprintAssignable, Category="Battle|UI")
	FOnTurnOrderUpdated OnTurnOrderChanged;
	
	UPROPERTY(BlueprintAssignable, Category="Battle|UI")
	FOnTargetChanged OnTargetChanged;
	
	UPROPERTY(BlueprintAssignable, Category="Battle|Result")
	FOnBattleFinished OnBattleFinished;
	
	UPROPERTY(BlueprintReadOnly)
	EBattleResult FinalResult = EBattleResult::None;
	
	// 유닛들을 스폰하는 함수
	void SpawnBattleUnits();
	
	void SetState(EBattleState NewState);
	void HandleNewRound();
	void HandleWaitTurn();
	void HandleActionInput();
	void HandleTargetSelection();
	void HandleCheckCondition();
	
	UFUNCTION()
	void OnUnitActionComplete();

	UFUNCTION()
	void OnBattleSetupFinished();
	
	// 적 유닛 타겟팅 상태 시작 (공격 또는 스킬 사용에서 호출)
	void StartTargetSelection(USkillDataAsset* SelectedSkill);
	
	void UpdateSelectedTargetAndBroadcast();
	void CancelTargetSelection();
	
	// 타겟 변경 함수 (화살표 입력 시 호출)
	// TODO: ESC 입력 시 다시 행동 입력 UI 상태로 전환
	void MoveOnSelection(int32 Direction);
	
	// 타겟 확정 (엔터 또는 유닛 클릭 시 호출)
	void ConfirmTarget();
	
	// 전체/단일 공격에 따른 분기
	void RequestUseSkill(USkillDataAsset* Skill);
	
	UFUNCTION(BlueprintCallable)
	EBattleState GetCurrentState() const { return CurrentState; }
	
	UFUNCTION(BlueprintCallable)
	void SelectTarget(AActor* NewTarget);
	
	void FinishBattle(EBattleResult Result);
	
	int32 GetAliveUnitCount(const TArray<AActor*>& UnitList);
	
	// 현재 전투에 참여 중인 적 유닛들을 담는 배열 (턴 관리용)
	UPROPERTY()
	TArray<AActor*> SpawnedEnemies;
	
protected:
	void HandleEnemyAI(ABattleBaseUnit* EnemyUnit);
	
	EBattleState CurrentState;
	
	UPROPERTY()
	UBattleTurnManager* TurnManager;
	
	AActor* CurrentActionUnit;
	
	// 현재 전투에 참여 중인 플레이어 유닛들을 담는 배열
	UPROPERTY()
	TArray<AActor*> SpawnedAllies;
	
private:
	// 유닛 스폰의 공통적인 부분을 담당하는 함수
	ABattleBaseUnit* SpawningLogic(const FUnitSpawnInfo& UnitInfo);
	
	
	
	void RebuildAvailableTargetsByRule();
	const TArray<AActor*>& GetFriendlyUnitsFor(const ABattleBaseUnit* ActingUnit) const;
	const TArray<AActor*>& GetOpposingUnitsFor(const ABattleBaseUnit* ActingUnit) const;
	void AddAliveUnitsFrom(const TArray<AActor*>& Src, TArray<AActor*>& OutTargets) const;
	
	UPROPERTY()
	TArray<AActor*> AvailableTargets;
	
	int32 CurrentTargetIndex = 0; // 타겟 리스트에서 현재 몇 번째 유닛을 가리키고 있는지에 대한 인덱스
	
	UPROPERTY()
	USkillDataAsset* PendingSkill; // 예약된 스킬
	
	UPROPERTY()
	AActor* SelectedTarget; // 현재 플레이어가 선택한 적 유닛
	
	bool bBattleResolved = false;
	
};
