// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SpawnDataAsset.h"
#include "Subsystems/WorldSubsystem.h"
#include "BattleControlSubsystem.generated.h"

class ABattleBaseUnit;

UCLASS()
class PCUBE_API UBattleControlSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	
	// 적 유닛들을 스폰하는 함수
	// TODO: 아군 유닛들까지 스폰하는 함수로 변경될 가능성이 있기 때문에 네이밍 포괄적으로 했음
	void SpawnBattleUnits();
	
protected:
	// 현재 전투에 참여 중인 적 유닛들을 담는 배열 (턴 관리용)
	UPROPERTY()
	TArray<AActor*> SpawnedEnemies;
	
	// 현재 전투에 참여 중인 플레이어 유닛들을 담는 배열
	UPROPERTY()
	TArray<AActor*> SpawnedAllies;
	
private:
	// 유닛 스폰의 공통적인 부분을 담당하는 함수
	ABattleBaseUnit* SpawningLogic(const FUnitSpawnInfo& UnitInfo);
};
