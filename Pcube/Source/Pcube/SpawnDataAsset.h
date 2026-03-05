// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LootTypes.h"
#include "Engine/DataAsset.h"
#include "SpawnDataAsset.generated.h"

class UUnitDataAsset;

USTRUCT(BlueprintType)
struct FUnitSpawnInfo
{
	GENERATED_BODY()
	
	// json 파싱용 유닛 ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spawn")
	FString UnitID;
	
	// 실제 에셋 참조 (에디터 작업용으로, json 파싱 시 id를 통해 런타임 할당)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spawn")
	TSoftObjectPtr<UUnitDataAsset> UnitDataAsset;
	
	// BattleLevel에서 스폰될 좌표
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spawn")
	FVector  SpawnLocation;
	
	// 스폰된 유닛이 바라보는 방향
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spawn")
	FRotator SpawnRotation;
	
	// 스폰된 유닛의 스케일 조정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spawn")
	FVector SpawnScale = FVector(1.0f, 1.0f, 1.0f);
	
	// 아래는 배치 예시 (앞줄 2마리, 뒷줄 3마리)
	// Pos_1 Pos_2 Pos_3
	//	 Pos_4   Pos_5
	
	// Json 예시: {
	// "EncounterID": "Worm_Squad_01",
	// "EnemyGroup": [
	//   { "SlotName": "Pos_1", "UnitID": "DA_RectHand_A" },
	//   { "SlotName": "Pos_2", "UnitID": "DA_RectHand_B" }
	// ]
	//  }
	
	// 아래 구조체 코드는 이후에 json 파싱을 고려하여 작성됨 *****
	
	// string 값 = key 값
	// UPROPERTY(EditAnywhere, BlueprintReadWrite)
	// TMap<FString, UUnitDataAsset*> SpawnMap;
};

// 한 번의 encounter에서 등장할 적들의 그룹
USTRUCT(BlueprintType)
struct FEnemySpawnGroup
{
	GENERATED_BODY()
		
	// json 파싱용 encounter 고유 ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spawn")
	FName EncounterID;
	
	// 해당 그룹에 포함된 모든 적 유닛 리스트 (개체수 & 위치 정보)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spawn")
	TArray<FUnitSpawnInfo> EnemyList;
	
	// --- Loot ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Loot", meta=(ClampMin="0"))
	int32 LootRolls = 2;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Loot")
	TArray<FLootDropEntry> LootTable;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Loot")
	TArray<FLootStack> GuaranteedLoot;
};

UCLASS()
class PCUBE_API USpawnDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	// 유닛 배치 조합 (사용자 정의)
	// 상황에 맞는 index를 선택하여 배치
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Battle")
	TArray<FEnemySpawnGroup> SpawnGroups;
};