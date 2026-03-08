// Fill out your copyright notice in the Description page of Project Settings.


#include "BattleControlSubsystem.h"
#include "BattleAllyUnit.h"
#include "BattleInfoTransferSubsystem.h"
#include "UnitDataAsset.h"
#include "EquipmentSubsystem.h"

void UBattleControlSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
	
	// BattleTurnManager 생성
	if (TurnManager == nullptr)
	{
		TurnManager = NewObject<UBattleTurnManager>(this);
		UE_LOG(LogTemp, Log, TEXT("BattleControlSubsystem: TurnManager Created"));
	}
	
	// 현재 레벨이 L_Battle인지 확인 - 다른 월드에서 오작동 방지
	FString CurrentLevelName = InWorld.GetMapName();
	if (CurrentLevelName.Contains("L_Battle"))
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
	UEquipmentSubsystem* EquipmentSubsystem = GI ? GI->GetSubsystem<UEquipmentSubsystem>() : nullptr;
	
	if (!BattleInfoSubsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("BattleInfoTransferSubsystem is null"));
		return;
	}
	
	// 2. 아군 유닛 스폰
	// 장착 시스템 배열 준비(파티 수에 맞춤)
	if (EquipmentSubsystem)
	{
		EquipmentSubsystem->EnsurePartySize(BattleInfoSubsystem->BattleInfo.AlliesToSpawn.Num());
	}
	if (BattleInfoSubsystem->BattleInfo.AlliesToSpawn.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("아군 데이터가 비어있습니다!"));
	}
	
	for (int32 AllyIndex = 0; AllyIndex < BattleInfoSubsystem->BattleInfo.AlliesToSpawn.Num(); ++AllyIndex)
	{
		const FUnitSpawnInfo& AllyInfo = BattleInfoSubsystem->BattleInfo.AlliesToSpawn[AllyIndex];
		
		if (ABattleBaseUnit* NewAlly = SpawningLogic(AllyInfo))
		{
			SpawnedAllies.Add(NewAlly);
			
			// 파티 인덱스 지정
			NewAlly->PartyIndex = AllyIndex;

			// 무기(고정 장비) 초기 세팅: 유닛 데이터에 DefaultWeapon이 있고, 아직 장착되지 않았다면 장착
			if (EquipmentSubsystem && NewAlly->UnitData && NewAlly->UnitData->DefaultWeapon)
			{
				if (!EquipmentSubsystem->GetEquippedWeapon(AllyIndex))
				{
					EquipmentSubsystem->EquipWeapon(AllyIndex, NewAlly->UnitData->DefaultWeapon);
				}
			}

			// 무기 비주얼(캐릭터 소켓에 WeaponMesh 부착) 갱신 - 장착된 무기가 있든 없든 강제 동기화
			NewAlly->RefreshWeaponVisual();
			
			// 저장된 HP 적용
			const float SavedHP = BattleInfoSubsystem->GetSavedHP(AllyIndex);
			if (SavedHP >= 0.f)
			{
				const float MaxHP = NewAlly->GetMaxHP();
				NewAlly->CurrentHP = FMath::Clamp(SavedHP, 0.f, MaxHP);
				
				// UI 갱신
				NewAlly->OnHPChanged.Broadcast(NewAlly->CurrentHP, MaxHP);
				
				// 전투 중 죽었으면, 죽은 상태 유지
				// TODO: 이후에 스폰할 때 죽은 애니메이션 출력
				if (NewAlly->CurrentHP <= 0.f){ NewAlly->Die(); }
			}
			
			const int32 SavedSP = BattleInfoSubsystem->GetSavedSP(AllyIndex);
			if (SavedSP >= 0)
			{
				const int32 MaxSP = NewAlly->GetMaxSkillPoints();
				NewAlly->CurrentSkillPoints = FMath::Clamp(SavedSP, 0, MaxSP);
				
				// UI 갱신 - 스킬포인트 아이콘/수치 업데이트
				NewAlly->OnSkillPointsChanged.Broadcast(NewAlly->CurrentSkillPoints, MaxSP);
			}
			
			UE_LOG(LogTemp, Log, TEXT("아군 스폰 성공: %s (PartyIndex=%d SavedHP=%.1f SavedSP=%d/%d)"),
			*NewAlly->GetName(), AllyIndex, SavedHP, NewAlly->CurrentSkillPoints, NewAlly->GetMaxSkillPoints());
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
	
	bBattleResolved = false; // FinishBattle()을 한 번만 실행되게 설정
	
	UE_LOG(LogTemp, Warning, TEXT("[Battle] Spawned Allies=%d Enemies=%d -> Broadcast OnBattleUnitsSpawned"),
	SpawnedAllies.Num(), SpawnedEnemies.Num());
	OnBattleUnitsSpawned.Broadcast(SpawnedAllies, SpawnedEnemies);
	
	SetState(EBattleState::NewRound);
}

ABattleBaseUnit* UBattleControlSubsystem::SpawningLogic(const FUnitSpawnInfo& UnitInfo)
{
	UWorld* World = GetWorld();
	if (!World) return nullptr;

	UUnitDataAsset* UnitDataAsset = UnitInfo.UnitDataAsset.LoadSynchronous();
	if (!UnitDataAsset || !UnitDataAsset->BattleUnitClass)
	{
		return nullptr;
	}

	FTransform SpawnTransform(UnitInfo.SpawnRotation, UnitInfo.SpawnLocation, UnitInfo.SpawnScale);

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	ABattleBaseUnit* NewUnit = World->SpawnActor<ABattleBaseUnit>(
		UnitDataAsset->BattleUnitClass,
		SpawnTransform,
		SpawnParams
	);

	if (!NewUnit)
	{
		UE_LOG(LogTemp, Error, TEXT("SpawningLogic: Spawn failed. UnitData=%s"), *GetNameSafe(UnitDataAsset));
		return nullptr;
	}

	// 초기화
	NewUnit->InitUnit(UnitDataAsset);

	// ★ 바인딩은 여기서 딱 1회만
	if (!NewUnit->OnActionFinished.IsAlreadyBound(this, &UBattleControlSubsystem::OnUnitActionComplete))
	{
		NewUnit->OnActionFinished.AddDynamic(this, &UBattleControlSubsystem::OnUnitActionComplete);
	}

	return NewUnit;
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
	if (!TurnManager) return;
	
	TArray<AActor*> AllParticipants;
	AllParticipants.Append(SpawnedAllies);
	AllParticipants.Append(SpawnedEnemies);
	
	// 주사위 Roll & Sort
	TurnManager->InitNewRound(AllParticipants);
	
	//TArray<AActor*> SortedList = TurnManager->GetSortedActorList();
	
	if (OnTurnOrderChanged.IsBound())
	{
		const TArray<AActor*> RemainingParticipants = TurnManager->GetRemainingActorList();
		OnTurnOrderChanged.Broadcast(RemainingParticipants);
		UE_LOG(LogTemp, Log, TEXT("[TurnOrder] RoundStart remaining=%d"), RemainingParticipants.Num());
	}
	
	SetState(EBattleState::WaitTurn);
}

void UBattleControlSubsystem::HandleWaitTurn()
{
	if (!TurnManager) return;
	
	if (TurnManager->IsRoundFinished())
	{
		SetState(EBattleState::NewRound);
		return;
	}
	
	CurrentActionUnit = TurnManager->GetNextUnit();
	
	UE_LOG(LogTemp, Warning, TEXT("[WaitTurn] selected=%s"), *GetNameSafe(CurrentActionUnit));
	
	if (CurrentActionUnit) SetState(EBattleState::ActionInput);
	else SetState(EBattleState::NewRound);
}

void UBattleControlSubsystem::HandleActionInput()
{
	if (!TurnManager)
	{
		UE_LOG(LogTemp, Error, TEXT("BattleControlSubsystem: TurnManager is NULL in HandleActionInput!"));
		return; 
	}
	
	UE_LOG(LogTemp, Warning, TEXT("[ActionInput] unit=%s class=%s"),
	*GetNameSafe(CurrentActionUnit),
	*GetNameSafe(CurrentActionUnit ? CurrentActionUnit->GetClass() : nullptr));
	
	// TurnManager로부터 결정된 현재 유닛을 캐스팅
	ABattleBaseUnit* ActiveUnit = Cast<ABattleBaseUnit>(CurrentActionUnit);
	if (!ActiveUnit) return;
	
	// 스턴인 경우, UI/AI 실행 전에 스킵 처리
	if (ActiveUnit->IsStunned())
	{
		UE_LOG(LogTemp, Warning, TEXT("[Turn] %s is STUNNED -> skip this turn"), *ActiveUnit->GetName());
		
		SetState(EBattleState::ActionExecute);
		ActiveUnit->FinishAction(); // 스턴 1 감소됨 (by TicBuffDuration_OnActionEnd)
		return;
	}
	
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
	
	if (!IsValid(CandidateUnit) || CandidateUnit->IsDead())
	{
		CurrentTargetIndex = 0;
		Candidate = AvailableTargets[0];
	}
	
	SelectedTarget = Candidate;
	
	if (OnTargetChanged.IsBound())
	{
		OnTargetChanged.Broadcast(SelectedTarget);
	}
}

const TArray<AActor*>& UBattleControlSubsystem::GetFriendlyUnitsFor(const ABattleBaseUnit* ActingUnit) const
{
	const bool bActorIsAlly = ActingUnit && ActingUnit->IsA<ABattleAllyUnit>();
	return bActorIsAlly ? SpawnedAllies : SpawnedEnemies;
}

const TArray<AActor*>& UBattleControlSubsystem::GetOpposingUnitsFor(const ABattleBaseUnit* ActingUnit) const
{
	const bool bActorIsAlly = ActingUnit && ActingUnit->IsA<ABattleAllyUnit>();
	return bActorIsAlly ? SpawnedEnemies : SpawnedAllies;
}

void UBattleControlSubsystem::AddAliveUnitsFrom(const TArray<AActor*>& Src, TArray<AActor*>& OutTargets) const
{
	for (AActor* Actor : Src)
	{
		ABattleBaseUnit* Unit = Cast<ABattleBaseUnit>(Actor);
		if (IsValid(Unit) && !Unit->IsDead())
		{
			OutTargets.Add(Unit);
		}
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
	if (CurrentState != EBattleState::TargetSelection) return;
	
	AActor* PrevSelected = SelectedTarget;
	
	// 항상 갱신 -> 유효 타겟만 남기기
	RebuildAvailableTargetsByRule();
	
	if (AvailableTargets.Num() <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[TargetMove] No targets -> cancel"));
		CancelTargetSelection();
		SetState(EBattleState::CheckCondition);
		return;
	}
	
	// 이전 타겟이 있는 경우 인덱스를 그 위치로 보정
	if (IsValid(PrevSelected))
	{
		const int32 PrevIndex = AvailableTargets.IndexOfByKey(PrevSelected);
		if (PrevIndex != INDEX_NONE)
		{
			CurrentTargetIndex = PrevIndex;
			SelectedTarget = PrevSelected;
		}
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

void UBattleControlSubsystem::RebuildAvailableTargetsByRule()
{
	AvailableTargets.Empty();
	
	if (!IsValid(PendingSkill))
	{
		SelectedTarget = nullptr;
		CurrentTargetIndex = INDEX_NONE;
		return;
	}

	ABattleBaseUnit* ActingUnit = Cast<ABattleBaseUnit>(CurrentActionUnit);
	if (!IsValid(ActingUnit))
	{
		SelectedTarget = nullptr;
		CurrentTargetIndex = INDEX_NONE;
		return;
	}

	switch (PendingSkill->TargetType)
	{
	case ESkillTargetRule::SingleEnemy:
		AddAliveUnitsFrom(GetOpposingUnitsFor(ActingUnit), AvailableTargets);
		break;
		
	case ESkillTargetRule::SingleAlly:
		AddAliveUnitsFrom(GetFriendlyUnitsFor(ActingUnit), AvailableTargets);
		break;
		
	case ESkillTargetRule::Self:
		if (!ActingUnit->IsDead())
		{
			AvailableTargets.Add(ActingUnit);
		}
		break;
		
	case ESkillTargetRule::AllEnemis:
	case ESkillTargetRule::AllAllies:
	default:
		CancelTargetSelection();
		return;
	}
	
	if (!AvailableTargets.IsValidIndex(CurrentTargetIndex))
	{
		CurrentTargetIndex = 0;
	}
	
	if (!IsValid(SelectedTarget) || AvailableTargets.IndexOfByKey(SelectedTarget) == INDEX_NONE)
	{
		SelectedTarget = AvailableTargets.IsValidIndex(CurrentTargetIndex) ? AvailableTargets[CurrentTargetIndex] : nullptr;
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

	ABattleBaseUnit* Attacker = Cast<ABattleBaseUnit>(CurrentActionUnit);
	if (!IsValid(Attacker))
	{
		UE_LOG(LogTemp, Error, TEXT("ConfirmTarget: Attacker invalid"));
		CancelTargetSelection();
		SetState(EBattleState::WaitTurn);
		return;
	}

	// ★ 현재 선택된 타겟을 그대로 사용 (인덱스 재계산/재구성으로 꼬이는 것 방지)
	ABattleBaseUnit* TargetUnit = Cast<ABattleBaseUnit>(SelectedTarget);

	UE_LOG(LogTemp, Warning, TEXT("[Confirm] Selected=%s valid=%d hp=%.2f isDead=%d"),
		*GetNameSafe(TargetUnit),
		IsValid(TargetUnit),
		TargetUnit ? TargetUnit->CurrentHP : -999.f,
		TargetUnit ? (TargetUnit->IsDead() ? 1 : 0) : -1);

	if (!IsValid(TargetUnit))
	{
		UE_LOG(LogTemp, Warning, TEXT("ConfirmTarget: target invalid -> retry select"));
		UpdateSelectedTargetAndBroadcast();
		return;
	}

	if (TargetUnit->IsDead())
	{
		UE_LOG(LogTemp, Warning, TEXT("ConfirmTarget: target dead (hp=%.2f) -> retry select"), TargetUnit->CurrentHP);
		UpdateSelectedTargetAndBroadcast();
		return;
	}

	// (선택) 여기서 타겟 선택 상태 데이터 정리하면 다음 턴에 덜 꼬임
	USkillDataAsset* SkillToUse = PendingSkill;
	PendingSkill = nullptr;

	AvailableTargets.Empty();
	SelectedTarget = nullptr;
	CurrentTargetIndex = INDEX_NONE;

	SetState(EBattleState::ActionExecute);
	Attacker->ExecuteAction(SkillToUse, TargetUnit);
}

void UBattleControlSubsystem::RequestUseSkill(USkillDataAsset* Skill)
{
	if (CurrentState != EBattleState::ActionInput) return;
	
	ABattleBaseUnit* Attacker = Cast<ABattleBaseUnit>(CurrentActionUnit);
	if (!IsValid(Attacker) || !IsValid(Skill)) return;

	if (!Attacker->CanUseSkill(Skill))
	{
		UE_LOG(LogTemp, Warning, TEXT("[SkillPoint] Not enough points (blocked before selection)."));
		return;
	}

	switch (Skill->TargetType)
	{
	case ESkillTargetRule::SingleEnemy:
	case ESkillTargetRule::SingleAlly:
		StartTargetSelection(Skill);
		break;

	case ESkillTargetRule::AllEnemis:
		{
			TArray<AActor*> Targets;
			AddAliveUnitsFrom(GetOpposingUnitsFor(Attacker), Targets);
			if (Targets.Num() == 0)
			{
				SetState(EBattleState::CheckCondition);
				return;
			}

			OnTargetChanged.Broadcast(nullptr);
			SetState(EBattleState::ActionExecute);
			Attacker->ExecuteAction(Skill, Targets);
		}
		break;

	case ESkillTargetRule::AllAllies:
		{
			TArray<AActor*> Targets;
			AddAliveUnitsFrom(GetFriendlyUnitsFor(Attacker), Targets);
			if (Targets.Num() == 0)
			{
				SetState(EBattleState::CheckCondition);
				return;
			}

			OnTargetChanged.Broadcast(nullptr);
			SetState(EBattleState::ActionExecute);
			Attacker->ExecuteAction(Skill, Targets);
		}
		break;

	case ESkillTargetRule::Self:
		{
			TArray<AActor*> Targets;
			Targets.Add(Attacker);
			SetState(EBattleState::ActionExecute);
			Attacker->ExecuteAction(Skill, Targets);
		}
		break;

	default:
		StartTargetSelection(Skill);
		break;
	}
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
	RebuildAvailableTargetsByRule();
	
	if (AvailableTargets.Num() <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[TargetSelection] No valid enemy targets"));
		PendingSkill = nullptr;
		SetState(EBattleState::CheckCondition); // 또는 ActionUnput으로 복귀
		return;
	}
	
	CurrentTargetIndex = 0;
	SetState(EBattleState::TargetSelection);
	UpdateSelectedTargetAndBroadcast();
}

void UBattleControlSubsystem::OnUnitActionComplete()
{
	UE_LOG(LogTemp, Warning, TEXT("[ActionComplete] unit=%s state=%d"),
		*GetNameSafe(CurrentActionUnit), (int32)CurrentState);
	
	// 전투가 끝났으면 무시
	if (CurrentState == EBattleState::Finished) return;
	if (CurrentState != EBattleState::ActionExecute) return;
	
	if (TurnManager && IsValid(CurrentActionUnit))
	{
		TurnManager->MarkUnitActed(CurrentActionUnit);
		
		// 여기서 남은 행동 순서에 대한 UI 갱신도 같이 브로드캐스트 가능
		//OnTurnOrderChanged.Broadcast(TurnManager->GetRemainingActorList());
	}
	
	// UI 갱신 (행동한 유닛 제거 + 죽은 유닛 제거)
	if (OnTurnOrderChanged.IsBound() && TurnManager)
	{
		const TArray<AActor*> Remaining = TurnManager->GetRemainingActorList();
		OnTurnOrderChanged.Broadcast(Remaining);
		UE_LOG(LogTemp, Log, TEXT("[TurnOrder] AfterAction remaining=%d"), Remaining.Num());
	}
	
	// 한 유닛의 행동이 끝난 후 승패를 확인하고 다음 턴 진행
	SetState(EBattleState::CheckCondition);
}

void UBattleControlSubsystem::HandleEnemyAI(ABattleBaseUnit* ActingEnemyUnit)
{
	if (!IsValid(ActingEnemyUnit) || ActingEnemyUnit->IsDead() || !ActingEnemyUnit->UnitData)
	{
		UE_LOG(LogTemp, Warning, TEXT("[EnemyAI] Acting unit invalid/dead/no UnitData -> FinishAction"));
		if (IsValid(ActingEnemyUnit))
		{
			ActingEnemyUnit->FinishAction();
		}
		return;
	}

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

	if (AliveAllies.Num() <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[EnemyAI] No alive allies -> FinishAction"));
		ActingEnemyUnit->FinishAction();
		return;
	}

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

	SetState(EBattleState::ActionExecute);

	if (!ChosenSkill)
	{
		ABattleBaseUnit* TargetAlly = AliveAllies[FMath::RandRange(0, AliveAllies.Num() - 1)];
		UE_LOG(LogTemp, Warning, TEXT("[EnemyAI] No valid skill -> BasicAttack fallback"));
		if (OnTargetChanged.IsBound())
		{
			OnTargetChanged.Broadcast(TargetAlly);
		}
		ActingEnemyUnit->BasicAttack(TargetAlly);
		ActingEnemyUnit->FinishAction();
		return;
	}

	TArray<AActor*> Candidates;
	switch (ChosenSkill->TargetType)
	{
	case ESkillTargetRule::SingleEnemy:
		AddAliveUnitsFrom(GetOpposingUnitsFor(ActingEnemyUnit), Candidates);
		break;
	case ESkillTargetRule::SingleAlly:
		AddAliveUnitsFrom(GetFriendlyUnitsFor(ActingEnemyUnit), Candidates);
		break;
	case ESkillTargetRule::AllEnemis:
		AddAliveUnitsFrom(GetOpposingUnitsFor(ActingEnemyUnit), Candidates);
		break;
	case ESkillTargetRule::AllAllies:
		AddAliveUnitsFrom(GetFriendlyUnitsFor(ActingEnemyUnit), Candidates);
		break;
	case ESkillTargetRule::Self:
		Candidates.Add(ActingEnemyUnit);
		break;
	default:
		break;
	}

	if (Candidates.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[EnemyAI] No valid targets -> FinishAction"));
		ActingEnemyUnit->FinishAction();
		return;
	}

	if (ChosenSkill->TargetType == ESkillTargetRule::SingleEnemy ||
		ChosenSkill->TargetType == ESkillTargetRule::SingleAlly)
	{
		ABattleBaseUnit* TargetUnit = Cast<ABattleBaseUnit>(Candidates[FMath::RandRange(0, Candidates.Num() - 1)]);
		if (!IsValid(TargetUnit))
		{
			ActingEnemyUnit->FinishAction();
			return;
		}

		if (OnTargetChanged.IsBound())
		{
			OnTargetChanged.Broadcast(TargetUnit);
		}

		UE_LOG(LogTemp, Warning, TEXT("[EnemyAI] %s uses %s on %s"),
			*ActingEnemyUnit->GetName(), *GetNameSafe(ChosenSkill), *TargetUnit->GetName());

		ActingEnemyUnit->ExecuteAction(ChosenSkill, TargetUnit);
		return;
	}

	if (OnTargetChanged.IsBound())
	{
		OnTargetChanged.Broadcast(nullptr);
	}

	UE_LOG(LogTemp, Warning, TEXT("[EnemyAI] %s uses %s on %d targets"),
		*ActingEnemyUnit->GetName(), *GetNameSafe(ChosenSkill), Candidates.Num());

	ActingEnemyUnit->ExecuteAction(ChosenSkill, Candidates);
}

int32 UBattleControlSubsystem::GetAliveUnitCount(const TArray<AActor*>& UnitList)
{
	int32 count = 0;
	
	for (AActor* Actor : UnitList)
	{
		ABattleBaseUnit* Unit = Cast<ABattleBaseUnit>(Actor);
		
		if (IsValid(Unit) && !Unit->IsDead())
		{
			count++;
		}
	}
	return count;
}

void UBattleControlSubsystem::FinishBattle(EBattleResult Result)
{
	if (bBattleResolved)
	{
		UE_LOG(LogTemp, Warning, TEXT("[FinishBattle] Duplicate ignored. Result=%d"), (int32)Result);
		return;
	}
	bBattleResolved = true;
	
	// 현재 아군 유닛들의 HP/SP 저장
	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			if (UBattleInfoTransferSubsystem* TransferSubsystem = GI->GetSubsystem<UBattleInfoTransferSubsystem>())
			{
				TransferSubsystem->EnsureAllyRuntimeSize(SpawnedAllies.Num());
				
				for (AActor* AllyActor : SpawnedAllies)
				{
					ABattleBaseUnit* Unit = Cast<ABattleBaseUnit>(AllyActor);
					if (!IsValid(Unit)) continue;
					
					if (Unit->PartyIndex != INDEX_NONE)
					{
						TransferSubsystem->SetSavedHP(Unit->PartyIndex, Unit->CurrentHP);					// 현재 HP 저장
						TransferSubsystem->SetSavedSP(Unit->PartyIndex, Unit->CurrentSkillPoints);	// 현재 SP 저장
						
						UE_LOG(LogTemp, Warning, TEXT("[PartySP] Saved idx=%d sp=%d/%d"),
							Unit->PartyIndex, Unit->CurrentSkillPoints, Unit->GetMaxSkillPoints());
					}
				}
			}
		}
	}
	
	FinalResult = Result;
	SetState(EBattleState::Finished);
	
	UE_LOG(LogTemp, Warning, TEXT("[BattleFinished] Result=%d"), (int32)Result);
	if (OnBattleFinished.IsBound())
	{
		OnBattleFinished.Broadcast(Result);
	}
}
