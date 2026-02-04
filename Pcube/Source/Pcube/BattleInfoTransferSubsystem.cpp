// Fill out your copyright notice in the Description page of Project Settings.


#include "BattleInfoTransferSubsystem.h"
#include "BattleProjectSettings.h"

void UBattleInfoTransferSubsystem::InitEnemyBattleInfo(const TArray<FUnitSpawnInfo>& NewSpawnEnemies)
{
	BattleInfo.EnemiesToSpawn.Empty();
	BattleInfo.EnemiesToSpawn = NewSpawnEnemies;
}

void UBattleInfoTransferSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	const UBattleProjectSettings* Settings = GetDefault<UBattleProjectSettings>();
	if (Settings)
	{
		if (!Settings->DefaultAllyPartyAsset.IsNull())
		{
			UAllyPartyDataAsset* AllyPartyData = Settings->DefaultAllyPartyAsset.LoadSynchronous();
			if (AllyPartyData)
			{
				InitializeDefaultAllyParty(AllyPartyData);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("BattleSubsystem: 데이터 에셋 로드 실패! (패키징 설정 확인 필요)"));
			}
		}
	}
	
	
	// if (Settings && !Settings->DefaultAllyPartyAsset.IsNull())
	// {
	// 	UAllyPartyDataAsset* AllyPartyData = Settings->DefaultAllyPartyAsset.LoadSynchronous();
	// 	InitializeDefaultAllyParty(AllyPartyData);
	// }
}

void UBattleInfoTransferSubsystem::InitializeDefaultAllyParty(UAllyPartyDataAsset* DefaultAllyPartyData)
{
	// TODO: 아래 내용들은 복붙한 것들. 수정 필요
	if (!DefaultAllyPartyData) return;

	BattleInfo.AlliesToSpawn.Empty();
    
	// 데이터 에셋에 등록된 멤버들을 FUnitSpawnInfo 구조체로 변환
	for (const FUnitSpawnInfo& MemberInfo : DefaultAllyPartyData->DefaultPartyMembers)
	{
		if (MemberInfo.UnitDataAsset.IsNull())
		{
			UE_LOG(LogTemp, Error, TEXT("멤버 중 유닛 데이터 에셋이 비어 있는 유닛이 있습니다! *****"));
			continue;
		}
		BattleInfo.AlliesToSpawn.Add(MemberInfo);
	}
}

// TODO: 저장할 데이터가 많아지는 경우, SaveGameManagerSubsystem을 따로 만들어서 저장/로드 로직 분리 필요