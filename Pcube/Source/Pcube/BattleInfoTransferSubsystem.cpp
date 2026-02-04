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
			UE_LOG(LogTemp, Log, TEXT("로드 시도 경로: %s"), *Settings->DefaultAllyPartyAsset.ToString());
			
			UAllyPartyDataAsset* AllyPartyData = Settings->DefaultAllyPartyAsset.LoadSynchronous();
			if (AllyPartyData)
			{
				InitializeDefaultAllyParty(AllyPartyData);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("BattleSubsystem: 데이터 에셋 로드 실패! 경로: %s"), *Settings->DefaultAllyPartyAsset.ToSoftObjectPath().ToString());
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("BattleSubsystem: 설정에 등록된 에셋이 Null입니다."));
		}
	}
}

void UBattleInfoTransferSubsystem::InitializeDefaultAllyParty(UAllyPartyDataAsset* DefaultAllyPartyData)
{
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