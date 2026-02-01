// Fill out your copyright notice in the Description page of Project Settings.


#include "BattleControlSubsystem.h"
#include "BattleInfoTransferSubsystem.h"
#include "UnitDataAsset.h"
#include "Kismet/GameplayStatics.h"

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
	UE_LOG(LogTemp, Error, TEXT("SpawnBattleUnits 정상 호출!"));
	
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	
	UGameInstance* GI = World->GetGameInstance();
	if (!GI)
	{
		return;
	}
	
	// 1. BattleInfoTransferSubsystem에서 데이터 가져오기
	UBattleInfoTransferSubsystem* BattleInfoSubsystem = GI->GetSubsystem<UBattleInfoTransferSubsystem>();
	if (!BattleInfoSubsystem || BattleInfoSubsystem->BattleInfo.EnemiesToSpawn.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("스폰할 적 데이터가 없습니다!"));
		return;
	}
	
	// 2. 적 유닛 스폰 루프
	for (const FEnemySpawnInfo& SpawnInfo : BattleInfoSubsystem->BattleInfo.EnemiesToSpawn)
	{
		// 소프트 포인터 로드 (데이터 에셋 로드 확인)
		UUnitDataAsset* UnitDataAsset = SpawnInfo.UnitDataAsset.LoadSynchronous();
		
		if (UnitDataAsset && UnitDataAsset->BattleUnitClass)
		{
			FTransform SpawnTransform(
				SpawnInfo.SpawnRotation,
				SpawnInfo.SpawnLocation,
				SpawnInfo.SpawnScale
			);
			
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
			
			// 실제 액터 스폰
			AActor* NewEnemy = World->SpawnActor<AActor>(
				UnitDataAsset->BattleUnitClass,
				SpawnTransform,
				SpawnParams
			);
			
			if (NewEnemy)
			{
				ABattleEnemyUnit* EnemyUnit = Cast<ABattleEnemyUnit>(NewEnemy);
				if (EnemyUnit)
				{
					// 데이터 에셋을 전달하여 메시와 애니메이션 할당
					EnemyUnit->InitUnit(UnitDataAsset);
					SpawnedEnemies.Add(EnemyUnit);
				}
				
				//SpawnedEnemies.Add(NewEnemy);
				UE_LOG(LogTemp, Log, TEXT("적 스폰 성공: %s"), *EnemyUnit->GetName());
				
				// TODO: 유닛에 스탯 주입 or 초기화 로직 작성 가능 (다른 클래스에서 로직을 불러올 수도 있을 듯?)
				
			}
		}
	}
	
	// 3. TODO: 플레이어 유닛들 스폰 로직?
	
	// 4. TODO: 스폰 완료 후 턴 매니저에 시작 알림
}
