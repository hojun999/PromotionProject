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

void UBattleInfoTransferSubsystem::SetPendingEncounter(FName EncounterID, FName ReturnWorldLevel)
{
	PendingEncounter.EncounterID = EncounterID;
	PendingEncounter.ReturnWorldLevel = ReturnWorldLevel;
	
	UE_LOG(LogTemp, Warning, TEXT("[Encounter] Pending set: %s return=%s"),
		*EncounterID.ToString(), *ReturnWorldLevel.ToString());
}

void UBattleInfoTransferSubsystem::SetReturnPoint(FName SourceWorldLevel, const FVector& Loc, const FRotator& Rot)
{
	BattleInfo.SourceLevelName = SourceWorldLevel;
	BattleInfo.ReturnLocation = Loc;
	BattleInfo.ReturnRotation = Rot;
	BattleInfo.bHasReturnPoint = true;
	
	UE_LOG(LogTemp, Log, TEXT("[ReturnPoint] Set: level=%s loc=%s rot=%s"),
		*SourceWorldLevel.ToString(),
		*Loc.ToString(),
		*Rot.ToString());
}

bool UBattleInfoTransferSubsystem::ConsumeReturnPoint(FVector& OutLoc, FRotator& OutRot)
{
	if (!BattleInfo.bHasReturnPoint)
	{
		return false;
	}
	
	OutLoc = BattleInfo.ReturnLocation;
	OutRot = BattleInfo.ReturnRotation;
	BattleInfo.bHasReturnPoint = false;
	
	UE_LOG(LogTemp, Log, TEXT("[ReturnPoint] Consumed: loc=%s rot=%s"),
		*OutLoc.ToString(),
		*OutRot.ToString());
	
	return true;
}

void UBattleInfoTransferSubsystem::ClearReturnPoint()
{
	BattleInfo.bHasReturnPoint = false;
	BattleInfo.ReturnLocation = FVector::ZeroVector;
	BattleInfo.ReturnRotation = FRotator::ZeroRotator;
}

FName UBattleInfoTransferSubsystem::GetReturnWorldLevelName() const
{
	// PendingEncounter가 있으면 우선
	if (PendingEncounter.ReturnWorldLevel != NAME_None)
	{
		return PendingEncounter.ReturnWorldLevel;
	}
	
	// 없는 경우 BattleInfo.SourceLevelName으로 fallback
	return BattleInfo.SourceLevelName;
}

void UBattleInfoTransferSubsystem::ClearPendingEncounter()
{
	PendingEncounter.EncounterID = NAME_None;
	PendingEncounter.ReturnWorldLevel = NAME_None;
}

void UBattleInfoTransferSubsystem::MarkLastEncounterDefeated()
{
	if (PendingEncounter.EncounterID != NAME_None)
	{
		DefeatedEncounterIds.Add(PendingEncounter.EncounterID);
		
		UE_LOG(LogTemp, Warning, TEXT("[Encounter] Defeated recorded: %s"),
			*PendingEncounter.EncounterID.ToString());
	}
}

bool UBattleInfoTransferSubsystem::IsEncounterDefeated(FName EncounterID) const
{
	return DefeatedEncounterIds.Contains(EncounterID);
}
