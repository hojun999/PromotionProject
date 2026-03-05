// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AllyPartyDataAsset.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SpawnDataAsset.h"
#include "LootTypes.h"
#include "Async/IAsyncTask.h"
#include "BattleInfoTransferSubsystem.generated.h"


class ABattleBaseUnit;
class UUnitDataAsset;

USTRUCT()
struct FPartyMemberRuntimeState
{
	GENERATED_BODY()
	
	
	UPROPERTY()
	float SavedHP = -1.f;	// -1이면 저장된 값 없음 -> 풀피로 스폰
	
	UPROPERTY()
	int32 SavedSP = -1;		// -1이면 저장값 없음 -> BaseSkillPoints로 스폰
};

USTRUCT(BlueprintType)
struct FBattleInfoStruct
{
	GENERATED_BODY()
	
	// 편성된 아군 리스트 및 배치 좌표 - 아군의 배치나 정보가 바뀌는 시점에 갱신됨 ***
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Battle")
	TArray<FUnitSpawnInfo> AlliesToSpawn;
	
	// BattleLevel에서 스폰할 적 유닛 리스트 (위치 정보 포함) - encounter 시점에 갱신됨 ***
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FUnitSpawnInfo> EnemiesToSpawn;
	
	// WorldLevel 복귀용 데이터
	// AllyUnit이 전투가 끝난 후 돌아갈 맵 이름
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Level")
	FName SourceLevelName;
	
	// 필드 맵에서 플레이어가 서 있던 위치와 방향 (WorldLevel 복귀 위치 정보)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Level")
	FVector ReturnLocation;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Level")
	FRotator ReturnRotation;
	
	bool bHasReturnPoint = false;
	
	// TODO: 배경 오브젝트에 대한 정보는 분리할 필요가 있어보임
	// 전투 배경 테마 정보
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Environment")
	int32 BattleBackgroundID;
};

USTRUCT(BlueprintType)
struct FEncounterContext
{
	GENERATED_BODY()
	
	UPROPERTY()
	FName EncounterID = NAME_None;
	
	UPROPERTY()
	FName ReturnWorldLevel = NAME_None;
};

USTRUCT()
struct FPendingLootConfig
{
	GENERATED_BODY()
	
	UPROPERTY()
	int32 LootRolls = 0;
	
	UPROPERTY()
	TArray<FLootDropEntry> LootTable;
	
	UPROPERTY()
	TArray<FLootStack> GuaranteedLoot;
};

UCLASS()
class PCUBE_API UBattleInfoTransferSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	void InitializeDefaultAllyParty(UAllyPartyDataAsset* DefaultAllyPartyData);
	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
	// 데이터 초기화를 위한 함수
	void InitEnemyBattleInfo(const TArray<FUnitSpawnInfo>& NewSpawnEnemies);
	
	// World Level에서 전투 진입 직전에 호출
	void SetPendingEncounter(FName EncounterID, FName ReturnWorldLevel);
	
	UFUNCTION(BlueprintCallable)
	void SetReturnPoint(FName SourceWorldLevel, const FVector& Loc, const FRotator& Rot);
	
	UFUNCTION(BlueprintCallable)
	bool ConsumeReturnPoint(FVector& OutLoc, FRotator& OutRot);
	
	UFUNCTION(BlueprintCallable)
	void ClearReturnPoint();
	
	// 전투 승리 후 호출
	void MarkLastEncounterDefeated();
	
	bool IsEncounterDefeated(FName EncounterID) const;
	
	UFUNCTION(BlueprintCallable)
	FName GetReturnWorldLevelName() const;
	
	// --- 아군 유닛 HP bar 갱신을 위한 함수들 ---
	void EnsureAllyRuntimeSize(int32 Num);
	float GetSavedHP(int32 PartyIndex) const;
	void SetSavedHP(int32 PartyIndex, float NewHP);
	
	UFUNCTION(BlueprintCallable)
	void ClearPendingEncounter();
	
	// 현재 활성화된 데이터 정보 - WorldEnemyUnit으로부터 전달됨
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Battle")
	FBattleInfoStruct BattleInfo;
	
	// 파티는 인덱스 기반으로 저장됨
	UPROPERTY()
	TArray<FPartyMemberRuntimeState> AllyRuntimeStates;

	
private:
	UPROPERTY()
	FEncounterContext PendingEncounter;
	
	UPROPERTY()
	TSet<FName> DefeatedEncounterIds;
	
	// --- SP Getter/Setter ---
public:
	int32 GetSavedSP(int32 PartyIndex) const;			// 저장된 SP 읽기 (-1이면 없는 상태)
	void SetSavedSP(int32 PartyIndex, int32 NewSP);		// 현재 SP 저장
	
	// --- Loot 관련 로직 ---
public:
	bool IsEncounterLooted(FName EncounterID) const; // 루팅 완료 여부 (시체 재생성 방지)
	bool TryGetEncounterLoot(FName EncounterID, TArray<FLootStack>& OutLoot) const; // 잔여 루팅 조회
	void SetEncounterLoot(FName EncounterID, TArray<FLootStack> Loot); // 잔여 루팅 저장
	void ClearEncounterLoot(FName EncounterID); // 잔여 루팅 제거
	void MarkEncounterLooted(FName EncounterID); // 루팅 완료 처리
	
	FName GetPendingEncounterID() const { return PendingEncounter.EncounterID; }
	void SetPendingLootConfig(int32 InLootRolls, const TArray<FLootDropEntry>& InTable, const TArray<FLootStack>& InGuaranteed);
	void ClearPendingLootConfig();
	TArray<FLootStack> GenerateLootFromPendingConfig(int32 MaxSlots = 8) const;
	
private:
	UPROPERTY()
	TMap<FName, FEncounterLootList> EncounterLootMap; // // EncounterID -> 잔여 루팅 목록
	
	UPROPERTY()
	TSet<FName> LootedEncounterIDs; // 완전히 루팅됨 - 시체 스폰 X
	
	UPROPERTY()
	FPendingLootConfig PendingLootConfig;
};
