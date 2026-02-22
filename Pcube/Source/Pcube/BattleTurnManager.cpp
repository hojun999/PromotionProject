// Fill out your copyright notice in the Description page of Project Settings.


#include "BattleTurnManager.h"
#include "BattleEnemyUnit.h"
#include "BattleAllyUnit.h"

void UBattleTurnManager::InitNewRound(const TArray<AActor*>& ParticipatingUnits)
{
	CurrentRound++;
	RoundArray.Empty();
	
	for (AActor* Unit : ParticipatingUnits)
	{
		if (!IsValid(Unit)) continue;
		
		if (ABattleBaseUnit* BU = Cast<ABattleBaseUnit>(Unit))
		{
			if (BU->IsDead())
			{
				continue;	// 죽은 유닛은 턴 표시 X
			}
		}
		
		FBattleTurnUnit TurnData;
		TurnData.UnitActor = Unit;
		TurnData.bHasActed = false;
		
		// 1. 턴 계산 공식: BaseSpeed + Random(1 ~ 8)
		int32 RandomRollValue = FMath::RandRange(1, 8);
		TurnData.InitialSpeedValue = FMath::RoundToInt(GetUnitSpeed(Unit)) + RandomRollValue;
		
		RoundArray.Add(TurnData);
	}
	
	// 2. InitialSpeedValue 기준 내림차순 정렬 (높은 유닛부터 행동)
	RoundArray.Sort([](const FBattleTurnUnit& A, const FBattleTurnUnit& B)
	{
		return A.InitialSpeedValue > B.InitialSpeedValue;
	});
	
	UE_LOG(LogTemp, Log, TEXT("--- Round %d Started ---"), CurrentRound);
}

AActor* UBattleTurnManager::GetNextUnit()
{
	for (FBattleTurnUnit& TurnData : RoundArray)
	{
		if (TurnData.bHasActed || !IsValid(TurnData.UnitActor)) continue;
		
		ABattleBaseUnit* Unit = Cast<ABattleBaseUnit>(TurnData.UnitActor);
		if (Unit && Unit->IsDead())
		{
			TurnData.bHasActed = true;
			continue;
		}
		
		return TurnData.UnitActor;
	}
	
	return nullptr; // 해당 라운드 행동 완료
}

TArray<AActor*> UBattleTurnManager::GetRemainingActorList() const
{
	TArray<AActor*> Out;
	for (const FBattleTurnUnit& TurnData : RoundArray)
	{
		if (TurnData.bHasActed) continue;
		if (!IsValid(TurnData.UnitActor)) continue;
		
		if (const ABattleBaseUnit* BU = Cast<ABattleBaseUnit>(TurnData.UnitActor))
		{
			if (BU->IsDead()) continue; // 죽은 유닛 즉시 제외
		}
		
		Out.Add(TurnData.UnitActor);
	}
	return Out;
}

void UBattleTurnManager::MarkUnitActed(AActor* UnitActor)
{
	if (!IsValid(UnitActor)) return;
	
	for (FBattleTurnUnit& TurnData : RoundArray)
	{
		if (TurnData.UnitActor == UnitActor)
		{
			TurnData.bHasActed = true;
			return;
		}
	}
}

bool UBattleTurnManager::IsRoundFinished() const
{
	// 한 개의 유닛이라도 행동을 안했다면,
	for (const FBattleTurnUnit& TurnData : RoundArray)
	{
		if (!TurnData.bHasActed)
		{
			return false; // false 반환
		}
	}
	return true;
}

float UBattleTurnManager::GetUnitSpeed(AActor* UnitActor) const
{
	if (ABattleBaseUnit* BaseUnit = Cast<ABattleBaseUnit>(UnitActor))
	{
		return BaseUnit->CurrentSpeed;
	}
	return 0.0f;
}

TArray<AActor*> UBattleTurnManager::GetSortedActorList() const
{
	TArray<AActor*> SortedActors;
	for (const FBattleTurnUnit& TurnData : RoundArray)
	{
		if (IsValid(TurnData.UnitActor))
		{
			SortedActors.Add(TurnData.UnitActor);
		}
	}
	return SortedActors;
}
