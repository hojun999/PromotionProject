// Fill out your copyright notice in the Description page of Project Settings.


#include "BattleInfoTransferSubsystem.h"
#include "BattleProjectSettings.h"

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
	for (int32 i = 0; i < DefaultAllyPartyData->DefaultMembers.Num(); ++i)
	{
		UUnitDataAsset* MemberAsset = DefaultAllyPartyData->DefaultMembers[i];
		if (!MemberAsset) continue;

		FUnitSpawnInfo NewAlly;
		NewAlly.UnitDataAsset = MemberAsset;
		NewAlly.UnitID = MemberAsset->UnitName; // 또는 에셋 내부에 정의된 고유 ID
        
		// 초기 배치 좌표 설정 (예: 아군 진영 좌측)
		NewAlly.SpawnLocation = FVector(-500.f, i * 200.f, 100.f);
		NewAlly.SpawnRotation = FRotator(0.f, 0.f, 0.f);
		NewAlly.SpawnScale = FVector(1.f);

		BattleInfo.AlliesToSpawn.Add(NewAlly);
	}
    
	UE_LOG(LogTemp, Warning, TEXT("아군 파티 %d명 준비 완료!"), BattleInfo.AlliesToSpawn.Num());
}

// TODO: 저장할 데이터가 많아지는 경우, SaveGameManagerSubsystem을 따로 만들어서 저장/로드 로직 분리 필요