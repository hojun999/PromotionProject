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
	if (Settings && !Settings->DefaultAllyPartyAsset.IsNull())
	{
		UAllyPartyDataAsset* AllyPartyData = Settings->DefaultAllyPartyAsset.LoadSynchronous();
		InitializeDefaultAllyParty(AllyPartyData);
	}
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
			UE_LOG(LogTemp, Error, TEXT("기본 아군 파티 데이터 에셋의 멤버가 비어있음! *****"));
			continue;
		}

		// 에디터에서 설정한 데이터 에셋의 정보가 그대로 전달됨
		UE_LOG(LogTemp, Error, TEXT("기본 아군 파티 데이터 에셋 로드 성공!"));
		BattleInfo.AlliesToSpawn.Add(MemberInfo);
	}
}

// TODO: 저장할 데이터가 많아지는 경우, SaveGameManagerSubsystem을 따로 만들어서 저장/로드 로직 분리 필요