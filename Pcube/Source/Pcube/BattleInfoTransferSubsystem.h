// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SpawnDataAsset.h"
#include "BattleInfoTransferSubsystem.generated.h"


class ABattleBaseUnit;
class UUnitDataAsset;

USTRUCT(BlueprintType)
struct FBattleInfoStruct
{
	GENERATED_BODY()
	
	// BattleLevel에서 스폰할 적 유닛 리스트 (위치 정보 포함)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FEnemySpawnInfo> EnemiesToSpawn;
	
	// WorldLevel 복귀용 데이터
	// 전투가 끝난 후 돌아갈 맵 이름
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Level")
	FName SourceLevelName;
	
	// 필드 맵에서 플레이어가 서 있던 위치와 방향 (WorldLevel 복귀 위치 정보)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Level")
	FVector ReturnLocation;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Level")
	FRotator ReturnRotation;
	
	// TODO: 배경 오브젝트에 대한 정보는 분리할 필요가 있어보임
	// 전투 배경 테마 정보
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Environment")
	int32 BattleBackgroundID;
};

UCLASS()
class PCUBE_API UBattleInfoTransferSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	// 현재 활성화된 데이터 정보 - WorldEnemyUnit으로부터 전달됨
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Battle")
	FBattleInfoStruct BattleInfo;
	
	// 데이터 초기화를 위한 함수
	void InitBattleInfo() { BattleInfo = FBattleInfoStruct(); }
};
