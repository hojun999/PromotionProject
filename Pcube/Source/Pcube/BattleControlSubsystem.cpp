// Fill out your copyright notice in the Description page of Project Settings.


#include "BattleControlSubsystem.h"
#include "BattleAllyUnit.h"
#include "BattleInfoTransferSubsystem.h"
#include "UnitDataAsset.h"
#include "EquipmentSubsystem.h"
#include "Components/CapsuleComponent.h"

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

			if (EquipmentSubsystem && NewAlly->UnitData)
			{
				EquipmentSubsystem->InitializeUnitLoadoutIfMissing(AllyIndex, NewAlly->UnitData);
			}

			NewAlly->RefreshBaseCombatStatsFromEquipment(true);
			NewAlly->RefreshEquipmentVisuals();
			NewAlly->SetEnemyHPBarVisible(false);
			
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
			SpawnedEnemies.Add(NewEnemy);
			NewEnemy->RefreshEquipmentVisuals();
			NewEnemy->SetEnemyHPBarVisible(true);
			NewEnemy->RefreshEnemyHPBar();
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

	// 비행 유닛은 레이캐스트 보정 없이 SpawnLocation.Z 그대로 사용 
	FVector AdjustedLocation = UnitInfo.SpawnLocation;
	if (!UnitDataAsset->bIsFlightUnit) 
	{
		const FVector TraceStart = AdjustedLocation + FVector(0.f, 0.f, 500.f);
		const FVector TraceEnd   = AdjustedLocation - FVector(0.f, 0.f, 500.f);
		FHitResult HitResult;
		FCollisionQueryParams QueryParams;
		QueryParams.bTraceComplex = false;
		if (World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic, QueryParams))
		{
			AdjustedLocation.Z = HitResult.ImpactPoint.Z;
			UE_LOG(LogTemp, Warning, TEXT("[SpawnFloor] %s | OriginalZ=%.1f ImpactZ=%.1f HitActor=%s"),
				*UnitInfo.UnitID, UnitInfo.SpawnLocation.Z, HitResult.ImpactPoint.Z, *GetNameSafe(HitResult.GetActor()));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[SpawnFloor] %s | 레이캐스트 미스 - OriginalZ=%.1f 그대로 사용"),
				*UnitInfo.UnitID, UnitInfo.SpawnLocation.Z);
		}
		UE_LOG(LogTemp, Warning, TEXT("[SpawnFloor] %s | FinalZ=%.1f"), *UnitInfo.UnitID, AdjustedLocation.Z);
	}
	else 
	{ 
		UE_LOG(LogTemp, Warning, TEXT("[SpawnFloor] %s | FlightUnit - SpawnZ=%.1f 그대로 사용"), *UnitInfo.UnitID, AdjustedLocation.Z); 
	} 

	FTransform SpawnTransform(UnitInfo.SpawnRotation, AdjustedLocation, UnitInfo.SpawnScale);

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn; 

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

	// 지상 유닛만 캡슐 HalfHeight 보정 적용 
	if (!UnitDataAsset->bIsFlightUnit) 
	{
		if (UCapsuleComponent* Capsule = NewUnit->GetCapsuleComponent()) 
		{
			const float HalfHeight = Capsule->GetScaledCapsuleHalfHeight(); 
			AdjustedLocation.Z += HalfHeight; 
			NewUnit->SetActorLocation(AdjustedLocation, false, nullptr, ETeleportType::TeleportPhysics); 
			UE_LOG(LogTemp, Warning, TEXT("[SpawnFloor] %s | HalfHeight=%.1f FinalZ=%.1f"), *UnitInfo.UnitID, HalfHeight, AdjustedLocation.Z); 
		}
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

	auto AddAliveUnits = [&](const TArray<AActor*>& Src)
	{
		for (AActor* Actor : Src)
		{
			ABattleBaseUnit* Unit = Cast<ABattleBaseUnit>(Actor);
			if (IsValid(Unit) && !Unit->IsDead())
			{
				AvailableTargets.Add(Unit);
			}
		}
	};

	switch (PendingSkill->TargetType)
	{
	case ESkillTargetRule::SingleEnemy:
		AddAliveUnits(SpawnedEnemies);
		break;
	
	case ESkillTargetRule::SingleAlly:
		AddAliveUnits(SpawnedAllies);
		break;
	
	case ESkillTargetRule::Self:
		{
			ABattleBaseUnit* SelfUnit = Cast<ABattleBaseUnit>(CurrentActionUnit);
			if (IsValid(SelfUnit) && !SelfUnit->IsDead())
			{
				AvailableTargets.Add(SelfUnit);
			}
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

	// SelectedTarget이 리스트에서 사라졌으면 현재 인덱스로 재선택
	if (!IsValid(SelectedTarget) || AvailableTargets.IndexOfByKey(SelectedTarget) == INDEX_NONE)
	{
		SelectedTarget = AvailableTargets[CurrentTargetIndex];
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
		return; // ActionInput 유지
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
			for (AActor* Actor : SpawnedEnemies)
			{
				ABattleBaseUnit* Unit = Cast<ABattleBaseUnit>(Actor);
				if (IsValid(Unit) && !Unit->IsDead()) Targets.Add(Unit);
			}
			if (Targets.Num() == 0)
			{
				SetState(EBattleState::CheckCondition);
				return;
			}
			
			// default camera 처리 - nullptr에 대한 처리 로직 따로 추가해야될듯?
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
	
	// 2. 스킬 먼저 선택 (타겟 분기 전에 필요) 
	USkillDataAsset* ChosenSkill = nullptr;
	const TArray<USkillDataAsset*>& SkillList = ActingEnemyUnit->UnitData->SkillList;

	TArray<USkillDataAsset*> ValidSkills;
	for (USkillDataAsset* S : SkillList)
	{
		if (IsValid(S)) ValidSkills.Add(S);
	}
	if (ValidSkills.Num() > 0)
	{
		ChosenSkill = ValidSkills[FMath::RandRange(0, ValidSkills.Num() - 1)];
	}

	// 3. TargetType 기준 타겟 결정 및 실행 
	const bool bTargetAll = ChosenSkill &&
		(ChosenSkill->TargetType == ESkillTargetRule::AllAllies); 

	SetState(EBattleState::ActionExecute);

	if (bTargetAll)
	{
		// 전체 아군 타겟 
		OnTargetChanged.Broadcast(nullptr); // 기본 카메라로 전환 

		TArray<AActor*> AllAllyActors;
		for (ABattleBaseUnit* Ally : AliveAllies)
		{
			AllAllyActors.Add(Ally);
		}

		UE_LOG(LogTemp, Warning, TEXT("[EnemyAI] %s uses %s on ALL (%d targets)"), 
			*ActingEnemyUnit->GetName(), *GetNameSafe(ChosenSkill), AllAllyActors.Num());

		ActingEnemyUnit->ExecuteAction(ChosenSkill, AllAllyActors); 
		return;
	}

	// 단일 타겟 
	ABattleBaseUnit* TargetAlly = AliveAllies[FMath::RandRange(0, AliveAllies.Num() - 1)];
	if (!IsValid(TargetAlly))
	{
		UE_LOG(LogTemp, Warning, TEXT("[EnemyAI] Target invalid -> FinishAction"));
		ActingEnemyUnit->FinishAction();
		return;
	}
	OnTargetChanged.Broadcast(TargetAlly);

	UE_LOG(LogTemp, Warning, TEXT("[EnemyAI] %s uses %s on %s"),
		*ActingEnemyUnit->GetName(), *GetNameSafe(ChosenSkill), *TargetAlly->GetName());

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

