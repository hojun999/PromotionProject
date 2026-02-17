// Fill out your copyright notice in the Description page of Project Settings.


#include "BattleControlSubsystem.h"

#include "BattleAllyUnit.h"
#include "BattleInfoTransferSubsystem.h"
#include "UnitDataAsset.h"

void UBattleControlSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
	
	// BattleTurnManager 생성
	if (TurnManager == nullptr)
	{
		TurnManager = NewObject<UBattleTurnManager>(this);
		UE_LOG(LogTemp, Log, TEXT("BattleControlSubsystem: TurnManager Created"));
	}
	
	// 현재 레벨이 BattleLevel인지 확인 - 다른 월드에서 오작동 방지
	FString CurrentLevelName = InWorld.GetMapName();
	if (CurrentLevelName.Contains("BattleLevel"))
	{
		// 즉시 실행 대신 타이머를 사용해 아주 잠깐 늦게 시작 (HUD가 바인딩할 시간을 줌)
		FTimerHandle TempHandle;
		InWorld.GetTimerManager().SetTimer(TempHandle, this, &UBattleControlSubsystem::SpawnBattleUnits, 0.1f, false);
	}
}

void UBattleControlSubsystem::SpawnBattleUnits()
{
	UWorld* World = GetWorld();
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	
	// 1. BattleInfoTransferSubsystem에서 데이터 가져오기
	UBattleInfoTransferSubsystem* BattleInfoSubsystem = GI ? GI->GetSubsystem<UBattleInfoTransferSubsystem>() : nullptr;
	
	if (!BattleInfoSubsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("BattleInfoTransferSubsystem is null"));
		return;
	}
	
	// 2. 아군 유닛 스폰
	if (BattleInfoSubsystem->BattleInfo.AlliesToSpawn.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("아군 데이터가 비어있습니다!"));
	}
	
	for (const FUnitSpawnInfo& AllyInfo : BattleInfoSubsystem->BattleInfo.AlliesToSpawn)
	{
		if (ABattleBaseUnit* NewAlly = SpawningLogic(AllyInfo))
		{
			// 필요하다면 아군 전용 로직 추가 가능 ---
			SpawnedAllies.Add(NewAlly);
			UE_LOG(LogTemp, Log, TEXT("아군 스폰 성공: %s"), *NewAlly->GetName());
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("아군 스폰 실패! 아군 Unit Data Asset을 불러오지 못했습니다."));
		}
	}
	
	// 3. 적 유닛 스폰
	if (BattleInfoSubsystem->BattleInfo.EnemiesToSpawn.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("전투에 생성할 적 데이터가 없습니다."));
	}
	
	for (const FUnitSpawnInfo& EnemyInfo : BattleInfoSubsystem->BattleInfo.EnemiesToSpawn)
	{
		if (ABattleBaseUnit* NewEnemy = SpawningLogic(EnemyInfo))
		{
			// 필요하다면 적 전용 로직 추가 가능 ---
			SpawnedEnemies.Add(NewEnemy);
			UE_LOG(LogTemp, Log, TEXT("적 스폰 성공: %s"), *NewEnemy->GetName());
		}
	}
	
	// 적 유닛 스폰 정보 초기화
	BattleInfoSubsystem->BattleInfo.EnemiesToSpawn.Empty();
	
	SetState(EBattleState::NewRound);
}

ABattleBaseUnit* UBattleControlSubsystem::SpawningLogic(const FUnitSpawnInfo& UnitInfo)
{
	UWorld* World = GetWorld();
	UUnitDataAsset* UnitDataAsset = UnitInfo.UnitDataAsset.LoadSynchronous();
	
	if (!UnitDataAsset || !UnitDataAsset->BattleUnitClass)
	{
		return nullptr;
	}
	
	FTransform SpawnTransform(UnitInfo.SpawnRotation, UnitInfo.SpawnLocation, UnitInfo.SpawnScale);
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	
	// battle 유닛들의 공통 부모인 ABattleBaseUnit으로 스폰 및 캐스팅
	ABattleBaseUnit* NewUnit = World->SpawnActor<ABattleBaseUnit>(
		UnitDataAsset->BattleUnitClass,
		SpawnTransform,
		SpawnParams
		);
	
	if (NewUnit)
	{
		// 데이터 에셋 할당 및 초기화 (메시, 기본 스탯 등)
		NewUnit->InitUnit(UnitDataAsset);
		
		// 구독 신청
		// 유닛의 OnActionFinished가 사용되면 BattleControlSubsystem에서 OnUnitActionComplete 함수 실행
		NewUnit->OnActionFinished.AddDynamic(this, &UBattleControlSubsystem::OnUnitActionComplete);
	}
	
	return NewUnit;
}

void UBattleControlSubsystem::OnBattleSetupFinished()
{
	// 스폰이 완료되었으므로 BattleInfoTransferSubsystem의 데이터 비워주기 - 이전 레벨에 의한 메모리 누수 방지
	if (UBattleInfoTransferSubsystem* TransferSub = GetWorld()->GetGameInstance()->GetSubsystem<UBattleInfoTransferSubsystem>())
	{
		TransferSub->BattleInfo.AlliesToSpawn.Empty();
		TransferSub->BattleInfo.EnemiesToSpawn.Empty();
		UE_LOG(LogTemp, Log, TEXT("Battle Data Cleared to prevent Memory Leak"));
	}
}

void UBattleControlSubsystem::SetState(EBattleState NewState)
{
	// 해당 내용 있으면 갱신이 안되는 부분 있는지 확인되면 지우기 ***
	if (CurrentState == NewState)
	{
		return;
	}
	
	CurrentState = NewState;

	if (OnBattleStateChanged.IsBound())
	{
		OnBattleStateChanged.Broadcast(CurrentState);
	}
	
	switch (CurrentState)
	{
	case EBattleState::NewRound:
		HandleNewRound(); break;
	case EBattleState::WaitTurn:
		HandleWaitTurn(); break;
	case EBattleState::ActionInput:
		HandleActionInput(); break;
	case EBattleState::TargetSelection:
		HandleTargetSelection(); break;
	case EBattleState::ActionExecute:
		/* 애니메이션 대기 */ break;
	case EBattleState::CheckCondition:
		HandleCheckCondition(); break;
	case EBattleState::Finished:
		UE_LOG(LogTemp, Log, TEXT("전투가 완전히 종료되었습니다.")); break;
	}
	
	UE_LOG(LogTemp, Warning, TEXT("Current State: %hhd"), CurrentState);
}

void UBattleControlSubsystem::HandleNewRound()
{
	if (!TurnManager)
	{
		UE_LOG(LogTemp, Error, TEXT("BattleControlSubsystem: TurnManager is NULL in HandleNewRound!"));
		return; 
	}
	
	TArray<AActor*> AllParticipants;
	AllParticipants.Append(SpawnedAllies);
	AllParticipants.Append(SpawnedEnemies);
	
	// 주사위 Roll & Sort
	TurnManager->InitNewRound(AllParticipants);
	
	TArray<AActor*> SortedList = TurnManager->GetSortedActorList();
	
	if (OnTurnOrderChanged.IsBound())
	{
		OnTurnOrderChanged.Broadcast(SortedList);
		UE_LOG(LogTemp, Log, TEXT("BattleSubsystem: Turn Order Broadcasted (%d units)"), SortedList.Num());
	}
	
	SetState(EBattleState::WaitTurn);
}

void UBattleControlSubsystem::HandleWaitTurn()
{
	if (!TurnManager)
	{
		UE_LOG(LogTemp, Error, TEXT("BattleControlSubsystem: TurnManager is NULL in HandleWaitTurn!"));
		return; 
	}
	
	// 라운드 종료 여부 확인
	if (TurnManager->IsRoundFinished())
	{
		SetState(EBattleState::NewRound);
		return;
	}
	
	CurrentActionUnit = TurnManager->GetNextUnit();
	
	if (CurrentActionUnit)
	{
		SetState(EBattleState::ActionInput);
	}
}

void UBattleControlSubsystem::HandleActionInput()
{
	if (!TurnManager)
	{
		UE_LOG(LogTemp, Error, TEXT("BattleControlSubsystem: TurnManager is NULL in HandleActionInput!"));
		return; 
	}
	
	// TurnManager로부터 결정된 현재 유닛을 캐스팅
	ABattleBaseUnit* ActiveUnit = Cast<ABattleBaseUnit>(CurrentActionUnit);
	if (!ActiveUnit) return;
	
	// UI에 현재 턴 유닛 알림
	OnTurnUnitChanged.Broadcast(ActiveUnit);
	
		
	if (ActiveUnit->IsA<ABattleAllyUnit>()) // 플레이어 유닛인 경우
	{
		// UI를 띄우고 플레이어 입력 대기
		UE_LOG(LogTemp, Log, TEXT("[BattleControlSubsystem] 플레이어 입력 대기 중... (UI 활성화)"));
		return;
	}
	
	// 적 유닛인 경우
	// 적 AI 유닛 로직 실행 -> 애니메이션 재생 -> 애니메이션 종료 시 Notify 호출
	// 유닛이 행동을 마치면 OnActionFinished.Broadcast()를 호출하게 함
	
	UE_LOG(LogTemp, Log, TEXT("[Turn] Enemy turn -> AI execute"));
	HandleEnemyAI(ActiveUnit);
}

void UBattleControlSubsystem::UpdateSelectedTargetAndBroadcast()
{
	if (AvailableTargets.Num() <= 0)
	{
		SelectedTarget = nullptr;
		CurrentTargetIndex = INDEX_NONE;
		return;
	}
	
	if (!AvailableTargets.IsValidIndex(CurrentTargetIndex))
	{
		CurrentTargetIndex = 0;
	}
	
	AActor* Candidate = AvailableTargets[CurrentTargetIndex];
	ABattleBaseUnit* CandidateUnit = Cast<ABattleBaseUnit>(Candidate);
	
	// 후보가 죽었거나 invalid면 0번으로 강제(AvailableTargets는 살아있는 유닛만 있어야 하므로 거의 해당될 확률 x)
	if (!IsValid(CandidateUnit) || CandidateUnit->IsDead())
	{
		CurrentTargetIndex = 0;
		Candidate = AvailableTargets[0];
	}
	
	SelectedTarget = Candidate;
	
	if (OnTargetChanged.IsBound() && IsValid(SelectedTarget))
	{
		OnTargetChanged.Broadcast(SelectedTarget);
	}
}


void UBattleControlSubsystem::HandleTargetSelection()
{
	// 상태 진입 시점에 타겟 1회 Braodcast
	// HUD/카메라가 여기서 반응
	
	UpdateSelectedTargetAndBroadcast();
	
	if (!IsValid(SelectedTarget))
	{
		UE_LOG(LogTemp, Warning, TEXT("TargetSelection: No valid target. Returning to ActionInput."));
	}
	
}

void UBattleControlSubsystem::MoveOnSelection(int32 Direction)
{
	if (CurrentState != EBattleState::TargetSelection)
	{
		return;
	}
	
	// 항상 갱신 -> 유효 타겟만 남기기
	RebuildAvilableEnemyTargets();
	
	if (AvailableTargets.Num() <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[TargetMove] No targets -> cancel"));
		CancelTargetSelection();
		SetState(EBattleState::CheckCondition);
		return;
	}
	
	// Direction은 -1(이전), +1(이후)만 허용
	Direction = (Direction < 0) ? -1 : (Direction > 0 ? 1 : 0);
	if (Direction == 0) return;
	
	// 원형 이동
	const int32 Num = AvailableTargets.Num();
	CurrentTargetIndex = (CurrentTargetIndex + Direction + Num) % Num;
	
	UpdateSelectedTargetAndBroadcast();
	
	UE_LOG(LogTemp, Warning, TEXT("[TargetMove] idx=%d/%d target=%s valid=%d"),
	CurrentTargetIndex, AvailableTargets.Num(),
	*GetNameSafe(SelectedTarget),
	IsValid(SelectedTarget));
}

void UBattleControlSubsystem::RebuildAvilableEnemyTargets()
{
	AvailableTargets.Empty();
	
	for (AActor* Actor : SpawnedEnemies)
	{
		ABattleBaseUnit* Unit = Cast<ABattleBaseUnit>(Actor);
		if (IsValid(Unit) && !Unit->IsDead())
		{
			AvailableTargets.Add(Unit);
		}
	}
}

void UBattleControlSubsystem::ConfirmTarget()
{
	if (CurrentState != EBattleState::TargetSelection)
	{
		return;
	}
	
	if (!PendingSkill)
	{
		UE_LOG(LogTemp, Warning, TEXT("ConfirmTarget: PendingSkill is null"));
		CancelTargetSelection();
		return;
	}
	
	if (!IsValid(SelectedTarget))
	{
		UE_LOG(LogTemp, Warning, TEXT("ConfirmTarget: SelectedTarget invalid. Retrying selection."));
		UpdateSelectedTargetAndBroadcast();
		return;
	}
	
	// 확정 직전에도 타겟 최신화
	RebuildAvilableEnemyTargets();
	if (AvailableTargets.Num() <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("ConfirmTarget: No targets -> CheckCondition"));
		PendingSkill = nullptr;
		SetState(EBattleState::CheckCondition);
		return;
	}
	
	// 현재 인덱스가 범위 밖이면 0으로
	if (!AvailableTargets.IsValidIndex(CurrentTargetIndex))
	{
		CurrentTargetIndex = 0;
	}
	
	SelectedTarget = AvailableTargets[CurrentTargetIndex];
	ABattleBaseUnit* TargetUnit = Cast<ABattleBaseUnit>(SelectedTarget);
	if (!IsValid(TargetUnit) || !TargetUnit->IsDead())
	{
		UE_LOG(LogTemp, Warning, TEXT("ConfirmTarget: target invalid/dead -> retry select"));
		CurrentTargetIndex = 0;
		UpdateSelectedTargetAndBroadcast();
		return;
	}
	
	ABattleBaseUnit* Attacker = Cast<ABattleBaseUnit>(CurrentActionUnit);
	if (!IsValid(Attacker))
	{
		UE_LOG(LogTemp, Error, TEXT("ConfirmTarget: CurrentActionUnit invalid"));
		CancelTargetSelection();
		SetState(EBattleState::WaitTurn);
		return;
	}
	
	// 액션 실행
	USkillDataAsset* SkillToUse = PendingSkill;
	AActor* TargetToUse = SelectedTarget;
	
	// 선택 상태 데이터 정리
	PendingSkill = nullptr;
	AvailableTargets.Empty();
	SelectedTarget = nullptr;
	CurrentTargetIndex = INDEX_NONE;
	
	SetState(EBattleState::ActionExecute);
	Attacker->ExecuteAction(SkillToUse, TargetToUse);
}

void UBattleControlSubsystem::SelectTarget(AActor* NewTarget)
{
	if (CurrentState != EBattleState::TargetSelection) return;
	
	if (!IsValid(NewTarget)) return;
	
	// AvailableTargets 안에 있는 적만 사용
	const int32 Index = AvailableTargets.IndexOfByKey(NewTarget);
	if (Index == INDEX_NONE)
	{
		UE_LOG(LogTemp, Warning, TEXT("SelectTarget: Clicked actor is not in AvailableTargets: %s"),
			*NewTarget->GetName());
		return;
	}
	
	CurrentTargetIndex = Index;
	SelectedTarget = NewTarget;
	
	UpdateSelectedTargetAndBroadcast();
}

void UBattleControlSubsystem::CancelTargetSelection()
{
	PendingSkill = nullptr;
	AvailableTargets.Empty();
	SelectedTarget = nullptr;
	CurrentTargetIndex = INDEX_NONE;
	
	// 플레이어 입력 상태로 복귀
	SetState(EBattleState::ActionInput);
}

void UBattleControlSubsystem::HandleCheckCondition()
{
	if (!TurnManager)
	{
		UE_LOG(LogTemp, Error, TEXT("BattleControlSubsystem: TurnManager is NULL in HandleCheckCondition!"));
		return; 
	}
	
	// 살아있는 유닛 수 확인
	const int32 AliveAllies = GetAliveUnitCount(SpawnedAllies);
	const int32 AliveEnemies = GetAliveUnitCount(SpawnedEnemies);
	
	UE_LOG(LogTemp, Log, TEXT("생존 확인 - 아군: %d, 적군: %d"), AliveAllies, AliveEnemies);
	
	// 승리 판정: 적이 모두 쓰러졌을 때
	if (AliveEnemies <= 0)
	{
		FinishBattle(EBattleResult::Victory);
		// TODO: 승리 UI 출력 및 보상 획득 로직(World의 적 유닛 사체로 변경) 연결
		return;
	}
	
	// 패배 판정: 아군이 모두 쓰러졌을 때
	if (AliveAllies <= 0)
	{
		FinishBattle(EBattleResult::Defeat);
		// TODO: 게임 오버 UI 출력 또는 메인 메뉴 레벨로 이동
		return;
	}
	
	// 전투 지속: 결과가 나지 않은 경우 다음 유닛의 턴 대기
	UE_LOG(LogTemp, Log, TEXT("전투 지속 - 다음 유닛을 결정합니다."));
	SetState(EBattleState::WaitTurn);
}

void UBattleControlSubsystem::StartTargetSelection(USkillDataAsset* SelectedSkill)
{
	PendingSkill = SelectedSkill;
	
	// 타겟 목록 갱신 -> 죽은 대상 제거
	RebuildAvilableEnemyTargets();
	
	if (AvailableTargets.Num() <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[TargetSelection] No valid enemy targets"));
		PendingSkill = nullptr;
		SetState(EBattleState::TargetSelection); // 또는 ActionUnput으로 복귀
		return;
	}
	
	CurrentTargetIndex = 0;
	SetState(EBattleState::TargetSelection);
	
	UpdateSelectedTargetAndBroadcast();
}

void UBattleControlSubsystem::OnUnitActionComplete()
{
	UE_LOG(LogTemp, Warning, TEXT("서브시스템: 유닛의 행동 종료 신호를 수신했습니다. 다음 턴으로 넘어갑니다."));
	
	// 한 유닛의 행동이 끝난 후 승패를 확인하고 다음 턴 진행
	SetState(EBattleState::CheckCondition);
}

void UBattleControlSubsystem::HandleEnemyAI(ABattleBaseUnit* ActingEnemyUnit)
{
	// 0. Acting 유닛이 이상하면 턴이 멈추지 않게 종료
	if (!IsValid(ActingEnemyUnit) || ActingEnemyUnit->IsDead() || !ActingEnemyUnit->UnitData)
	{
		UE_LOG(LogTemp, Warning, TEXT("[EnemyAI] Acting unit invalid/dead/no UnitData -> FinishAction"));
		if (IsValid(ActingEnemyUnit))
		{
			ActingEnemyUnit->FinishAction();
		}
		return;
	}
	
	// 1. 살아있는 아군 목록 만들기
	TArray<ABattleBaseUnit*> AliveAllies;
	AliveAllies.Reserve(SpawnedAllies.Num());
	
	for (AActor* Actor : SpawnedAllies)
	{
		ABattleBaseUnit* Ally = Cast<ABattleBaseUnit>(Actor);
		if (IsValid(Ally) && !Ally->IsDead())
		{
			AliveAllies.Add(Ally);
		}
	}
	
	// 아군이 없으면 -> 턴 종료 후 CheckCondition으로 전환
	if (AliveAllies.Num() <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[EnemyAI] No alive allies -> FinishAction"));
		ActingEnemyUnit->FinishAction();
		return;
	}
	
	// 2. 타겟 선택
	ABattleBaseUnit* TargetAlly = AliveAllies[FMath::RandRange(0, AliveAllies.Num() - 1)];
	if (!IsValid(TargetAlly))
	{
		UE_LOG(LogTemp, Warning, TEXT("[EnemyAI] Target invalid -> FinishAction"));
		ActingEnemyUnit->FinishAction();
		return;
	}
	
	// 카메라 전환 브로드캐스트
	if (OnTargetChanged.IsBound())
	{
		OnTargetChanged.Broadcast(TargetAlly);
	}
	
	// 3. 스킬 선택
	USkillDataAsset* ChosenSkill = nullptr;
	const TArray<USkillDataAsset*>& SkillList = ActingEnemyUnit->UnitData->SkillList;
	
	TArray<USkillDataAsset*> ValidSkills;
	for (USkillDataAsset* S : SkillList)
	{
		if (IsValid(S))
		{	
			ValidSkills.Add(S);
		}
	}
	
	if (ValidSkills.Num() > 0)
	{
		ChosenSkill = ValidSkills[FMath::RandRange(0, ValidSkills.Num() - 1)];
	}
	
	UE_LOG(LogTemp, Warning, TEXT("[EnemyAI] %s uses %s on %s"),
		*ActingEnemyUnit->GetName(), *GetNameSafe(ChosenSkill), *TargetAlly->GetName());
	
	// 4. 실행 - 스킬 없으면 기본공격 + FinishAction -> 턴 진행 보장
	SetState(EBattleState::ActionExecute);
	
	if (!ChosenSkill)
	{
		UE_LOG(LogTemp, Warning, TEXT("[EnemyAI] No valid skill -> BasicAttack fallback"));
		ActingEnemyUnit->BasicAttack(TargetAlly);
		ActingEnemyUnit->FinishAction();
		return;
	}
	
	ActingEnemyUnit->ExecuteAction(ChosenSkill, TargetAlly);
}

int32 UBattleControlSubsystem::GetAliveUnitCount(const TArray<AActor*>& UnitList)
{
	int32 count = 0;
	
	for (AActor* Actor : UnitList)
	{
		ABattleBaseUnit* Unit = Cast<ABattleBaseUnit>(Actor);
		
		// 유닛이 유효하고, HP > 0 인지 확인
		// TODO: IsDead()와 같은 함수 추가
		if (IsValid(Unit) && Unit->CurrentHP > 0.0f)
		{
			count++;
		}
	}
	return count;
}

void UBattleControlSubsystem::FinishBattle(EBattleResult Result)
{
	FinalResult = Result;
	SetState(EBattleState::Finished);
	
	UE_LOG(LogTemp, Warning, TEXT("[BattleFinished] Result=%d"), (int32)Result);
	
	if (OnBattleFinished.IsBound())
	{
		OnBattleFinished.Broadcast(Result);
	}
}
