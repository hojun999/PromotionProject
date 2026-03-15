// Fill out your copyright notice in the Description page of Project Settings.


#include "BattleInfoTransferSubsystem.h"
#include "BattleProjectSettings.h"
#include "ItemDataAsset.h"

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
	PartyUnitDataList.Empty();
    
	// 데이터 에셋에 등록된 멤버들을 FUnitSpawnInfo 구조체로 변환
	for (const FUnitSpawnInfo& MemberInfo : DefaultAllyPartyData->DefaultPartyMembers)
	{
		if (MemberInfo.UnitDataAsset.IsNull())
		{
			UE_LOG(LogTemp, Error, TEXT("멤버 중 유닛 데이터 에셋이 비어 있는 유닛이 있습니다! *****"));
			continue;
		}
		BattleInfo.AlliesToSpawn.Add(MemberInfo);
		PartyUnitDataList.Add(MemberInfo.UnitDataAsset);
	}
	
	EnsureAllyRuntimeSize(BattleInfo.AlliesToSpawn.Num());
}

void UBattleInfoTransferSubsystem::SetPendingEncounter(FName EncounterID, FName ReturnWorldLevel, bool bInIsBossEncounter)
{
	PendingEncounter.EncounterID = EncounterID;
	PendingEncounter.ReturnWorldLevel = ReturnWorldLevel;
	bIsBossEncounter = bInIsBossEncounter;
	
	UE_LOG(LogTemp, Warning, TEXT("[Encounter] Pending set: %s return=%s boss=%d"),
		*EncounterID.ToString(), *ReturnWorldLevel.ToString(), bInIsBossEncounter ? 1 : 0);
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
	
	ClearPendingLootConfig();
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

void UBattleInfoTransferSubsystem::EnsureAllyRuntimeSize(int32 Num)
{
	if (AllyRuntimeStates.Num() < Num)
	{
		const int32 Old = AllyRuntimeStates.Num();
		AllyRuntimeStates.SetNum(Num);
		
		for (int32 i = Old; i < Num; ++i)
		{
			AllyRuntimeStates[i].SavedHP = -1.f;
			AllyRuntimeStates[i].SavedSP = -1;
		}
	}
}

float UBattleInfoTransferSubsystem::GetSavedHP(int32 PartyIndex) const
{
	return AllyRuntimeStates.IsValidIndex(PartyIndex) ? AllyRuntimeStates[PartyIndex].SavedHP : -1.f;
}

void UBattleInfoTransferSubsystem::SetSavedHP(int32 PartyIndex, float NewHP)
{
	EnsureAllyRuntimeSize(PartyIndex + 1);
	AllyRuntimeStates[PartyIndex].SavedHP = NewHP;
}

float UBattleInfoTransferSubsystem::GetSavedMaxHP(int32 PartyIndex) const // 추가됨
{
	return AllyRuntimeStates.IsValidIndex(PartyIndex) ? AllyRuntimeStates[PartyIndex].SavedMaxHP : -1.f;
}

void UBattleInfoTransferSubsystem::SetSavedMaxHP(int32 PartyIndex, float NewMaxHP) // 추가됨
{
	EnsureAllyRuntimeSize(PartyIndex + 1);
	AllyRuntimeStates[PartyIndex].SavedMaxHP = NewMaxHP;
}

int32 UBattleInfoTransferSubsystem::GetSavedSP(int32 PartyIndex) const
{
	return AllyRuntimeStates.IsValidIndex(PartyIndex) ? AllyRuntimeStates[PartyIndex].SavedSP : -1;
}

void UBattleInfoTransferSubsystem::SetSavedSP(int32 PartyIndex, int32 NewSP)
{
	EnsureAllyRuntimeSize(PartyIndex + 1);
	AllyRuntimeStates[PartyIndex].SavedSP = NewSP;
}

bool UBattleInfoTransferSubsystem::IsEncounterLooted(FName EncounterID) const
{
	return LootedEncounterIDs.Contains(EncounterID);
}

bool UBattleInfoTransferSubsystem::TryGetEncounterLoot(FName EncounterID, TArray<FLootStack>& OutLoot) const
{
	if (const FEncounterLootList* Found = EncounterLootMap.Find(EncounterID))
	{
		OutLoot = Found->Items;
		return true;
	}
	return false;
}

void UBattleInfoTransferSubsystem::SetEncounterLoot(FName EncounterID, TArray<FLootStack> Loot)
{
	if (EncounterID == NAME_None) return;
	EncounterLootMap.FindOrAdd(EncounterID).Items = Loot;
}

void UBattleInfoTransferSubsystem::ClearEncounterLoot(FName EncounterID)
{
	EncounterLootMap.Remove(EncounterID);
}

void UBattleInfoTransferSubsystem::MarkEncounterLooted(FName EncounterID)
{
	if (EncounterID == NAME_None) return;
	LootedEncounterIDs.Add(EncounterID);
	ClearEncounterLoot(EncounterID);
}

void UBattleInfoTransferSubsystem::SetPendingLootConfig(int32 InLootRolls, const TArray<FLootDropEntry>& InTable, const TArray<FLootStack>& InGuaranteed)
{
	PendingLootConfig.LootRolls = FMath::Max(0, InLootRolls);
	PendingLootConfig.LootTable = InTable;
	PendingLootConfig.GuaranteedLoot = InGuaranteed;
}

void UBattleInfoTransferSubsystem::ClearPendingLootConfig()
{
	PendingLootConfig.LootRolls = 0;
	PendingLootConfig.LootTable.Reset();
	PendingLootConfig.GuaranteedLoot.Reset();
}

TArray<FLootStack> UBattleInfoTransferSubsystem::GenerateLootFromPendingConfig(int32 MaxSlots) const
{
	TArray<FLootStack> Result;
	
	auto Push = [&](UItemDataAsset* Item, int32 Count)
	{
		if (!Item || Count <= 0) return;
		if (Result.Num() >= MaxSlots) return;

		if (Item->bStackable)
		{
			// 동일 아이템 스택이 있으면 합치고, 없으면 새 스택 생성
			for (FLootStack& S : Result)
			{
				if (S.Item == Item)
				{
					S.Count += Count;
					return;
				}
			}

			FLootStack NewStack;
			NewStack.Item = Item;
			NewStack.Count = Count;
			Result.Add(NewStack);
		}
		else
		{
			// 비스택은 1개=1슬롯으로 쪼갬
			for (int32 i = 0; i < Count && Result.Num() < MaxSlots; ++i)
			{
				FLootStack NewStack;
				NewStack.Item = Item;
				NewStack.Count = 1;
				Result.Add(NewStack);
			}
		}
	};
;
	
	// 1. 보장 드랍 먼저
	for (const FLootStack& G : PendingLootConfig.GuaranteedLoot)
	{
		if (G.Item && G.Count > 0)
		{
			Push(G.Item, G.Count);
		}
		if (Result.Num() >= MaxSlots) return Result;
	}
	
	// 2. 가중치 테이블 준비
	float TotalWeight = 0.f;
	TArray<const FLootDropEntry*> Valid;
	for (const FLootDropEntry& E : PendingLootConfig.LootTable)
	{
		if (!E.Item || E.Weight <= 0.f) continue;
		Valid.Add(&E);
		TotalWeight += E.Weight;
	}

	if (Valid.Num() == 0 || TotalWeight <= 0.f || PendingLootConfig.LootRolls <= 0)
	{
		return Result;
	}

	// 3) 롤
	for (int32 r = 0; r < PendingLootConfig.LootRolls && Result.Num() < MaxSlots; ++r)
	{
		const float Pick = FMath::FRandRange(0.f, TotalWeight);

		float Acc = 0.f;
		const FLootDropEntry* Chosen = nullptr;
		for (const FLootDropEntry* E : Valid)
		{
			Acc += E->Weight;
			if (Pick <= Acc)
			{
				Chosen = E;
				break;
			}
		}
		if (!Chosen) Chosen = Valid.Last();

		const int32 MinC = FMath::Max(1, Chosen->MinCount);
		const int32 MaxC = FMath::Max(MinC, Chosen->MaxCount);
		const int32 Count = FMath::RandRange(MinC, MaxC);

		Push(Chosen->Item, Count);
	}

	return Result;
}

UUnitDataAsset* UBattleInfoTransferSubsystem::GetPartyUnitData(int32 PartyIndex) const 
{ 
	if (!PartyUnitDataList.IsValidIndex(PartyIndex)) return nullptr; 
	return PartyUnitDataList[PartyIndex].LoadSynchronous(); 
} 