// 1. GameInstance 서브시스템에 저장된 PatterIndex 선택
// 2. WorldLevel 로드
// 3. 선택된 패턴의 Placements를 순회하며 WorldEnemyUnit 스폰
// 4. 각 WorldEnemyUnit은 EncounterID 보관
// 5. 플레이어가 상호작용해서 전투 시작 시,
	// SapwnDataAsset에서 EncounterID에 해당하는 EnemySpawnGroup를 찾아서
	// BattleInfoTransferSubsystem->InitEnemyBattleInfo(Group.EnemyList)로 넘김
	// BattleLevel 오픈


#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "WorldEncounterPlacementAsset.generated.h"


USTRUCT(BlueprintType)
struct FWorldEncounterPlacement
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString EncounterID;		// SpawnDataAsset의 EncounterID와 동일
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTransform WorldTransform;	// 월드 좌표
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName SubLevelName;			// 어느 서브레벨에 속할지
};

USTRUCT(BlueprintType)
struct FWorldSpawnPattern
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FWorldEncounterPlacement> Placements;
};

UCLASS()
class PCUBE_API UWorldEncounterPlacementAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FWorldSpawnPattern> Patterns;
};

