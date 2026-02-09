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
	
	SetState(EBattleState::NewRound);
	// 4. TODO: 모든 유닛 스폰 완료 후 턴 매니저에 유닛 리스트 전달 및 전투 시작
	// StartBattle();
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
	case EBattleState::ActionExecute:
		/* 애니메이션 대기 */ break;
	case EBattleState::CheckCondition:
		HandleCheckCondition(); break;
	case EBattleState::Finished:
		UE_LOG(LogTemp, Log, TEXT("전투가 완전히 종료되었습니다.")); break;
	}
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
	
	
	// UI에 현재 누구의 턴인지 알림
	if (OnTurnUnitChanged.IsBound())
	{
		OnTurnUnitChanged.Broadcast(ActiveUnit);
	}
		
	if (ActiveUnit->IsA<ABattleAllyUnit>()) // 플레이어 유닛인 경우
	{
		// UI를 띄우고 플레이어 입력 대기
		UE_LOG(LogTemp, Log, TEXT("플레이어 입력 대기 중... (UI 활성화)"));
	}
	else // 적 유닛인 경우
	{
		// 적 AI 유닛 로직 실행 -> 애니메이션 재생 -> 애니메이션 종료 시 Notify 호출
		// 유닛이 행동을 마치면 OnActionFinished.Broadcast()를 호출하게 함
	}
	
}

void UBattleControlSubsystem::HandleCheckCondition()
{
	if (!TurnManager)
	{
		UE_LOG(LogTemp, Error, TEXT("BattleControlSubsystem: TurnManager is NULL in HandleCheckCondition!"));
		return; 
	}
	
	// 살아있는 유닛 수 확인
	int32 AliveAllies = GetAliveUnitCount(SpawnedAllies);
	int32 AliveEnemies = GetAliveUnitCount(SpawnedEnemies);
	
	UE_LOG(LogTemp, Log, TEXT("생존 확인 - 아군: %d, 적군: %d"), AliveAllies, AliveEnemies);
	
	// 승리 판정: 적이 모두 쓰러졌을 때
	if (AliveAllies <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("전투 승리!"));
		SetState(EBattleState::Finished);
		// TODO: 승리 UI 출력 및 보상 획득 로직(World의 적 유닛 사체로 변경) 연결
		return;
	}
	
	// 패배 판정: 아군이 모두 쓰러졌을 때
	if (AliveAllies <= 0)
	{
		UE_LOG(LogTemp, Error, TEXT("전투 패배..."));
		SetState(EBattleState::Finished);
		// TODO: 게임 오버 UI 출력 또는 메인 메뉴 레벨로 이동
		return;
	}
	
	// 전투 지속: 결과가 나지 않은 경우 다음 유닛의 턴 대기
	UE_LOG(LogTemp, Log, TEXT("전투 지속 - 다음 유닛을 결정합니다."));
	SetState(EBattleState::WaitTurn);
}

void UBattleControlSubsystem::OnUnitActionComplete()
{
	UE_LOG(LogTemp, Warning, TEXT("서브시스템: 유닛의 행동 종료 신호를 수신했습니다. 다음 턴으로 넘어갑니다."));
	
	// 한 유닛의 행동이 끝난 후 승패를 확인하고 다음 턴 진행
	SetState(EBattleState::CheckCondition);
}

void UBattleControlSubsystem::HandleEnemyAI(ABattleBaseUnit* EnemyUnit)
{
	if (!EnemyUnit || !EnemyUnit->UnitData) return;
	
	const TArray<USkillDataAsset*>& Skills = EnemyUnit->UnitData->SkillList;
	if (Skills.Num() == 0) return;
	
	// 1. 가중치 총합 계산
	float TotalWeight = 0.f;
	// for (auto* Skill : Skills) TotalWeight += Skill->SelectionWeight; // 스킬 에셋에 있는 가중치
	
	// 2. 가중치 기반 랜덤 선택
	float RandomValue = FMath::FRandRange(0.f, TotalWeight);
	USkillDataAsset* SelectedSkill = nullptr;
	float CurrentWeightSum = 0.f;
	
	for (auto* Skill : Skills)
	{
		// CurrentWeightSum += Skill->SelectionWeight;
		if (RandomValue <= CurrentWeightSum)
		{
			SelectedSkill = Skill;
			break;
		}
	}
	
	// 3. 타겟 결정 (살아있는 아군 중 랜덤)
	if (SpawnedAllies.Num() > 0)
	{
		int32 RandomIndex = FMath::RandRange(0, SpawnedAllies.Num() - 1);
		AActor* Target = SpawnedAllies[RandomIndex];
		
		// 4. 행동 실행 상태로 전환 (딜레이를 주어 카메라 연출 시간 확보)
		FTimerHandle ActionTimer;
		GetWorld()->GetTimerManager().SetTimer(ActionTimer, [this, EnemyUnit, SelectedSkill, Target]()
		{
			// 실제 유닛에게 스킬 실행 명령을 내리고 상태를 Execute로 변경
			// EnemyUnit->ExecuteSkill(SelectedSkill, Target);
			SetState(EBattleState::ActionExecute);
		}, 1.0f, false);
	}
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
