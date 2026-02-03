// Fill out your copyright notice in the Description page of Project Settings.


#include "BattleControlSubsystem.h"
#include "BattleInfoTransferSubsystem.h"
#include "UnitDataAsset.h"

void UBattleControlSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
	
	// 현재 레벨이 BattleLevel인지 확인 - 다른 월드에서 오작동 방지
	FString CurrentLevelName = InWorld.GetMapName();
	if (CurrentLevelName.Contains("BattleLevel"))
	{
		SpawnBattleUnits();
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
	}
	
	return NewUnit;
}
