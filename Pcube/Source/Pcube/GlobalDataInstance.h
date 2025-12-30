// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Templates/SubclassOf.h"
#include "GlobalDataInstance.generated.h"

/**
 * 
 */

class ABaseUnit;

USTRUCT(BlueprintType)
struct FBattleEncounterInfo
{
	GENERATED_BODY()
	
	// 전투에 등장할 적 클래스 목록
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Battle")
	TArray<TSubclassOf<ABaseUnit>> EnemyClasses;
	
	// 전투가 끝난 후 돌아갈 맵 이름
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Level")
	FName SourceLevelName;
	
	// 필드 맵에서 플레이어가 서 있던 위치와 방향
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Level")
	FVector ReturnLocation;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Level")
	FRotator ReturnRotation;
	
	// 전투 배경 테마
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Environment")
	int32 BattleBackgroundID;
};

UCLASS()
class PCUBE_API UGlobalDataInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	// 현재 활성화된 전투 정보 (레벨 이동 시 해당 데이터 참조)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat")
	FBattleEncounterInfo BattleInfo;
	
	void SetupBattle(const TArray<TSubclassOf<ABaseUnit>>& InEnemies, FVector InLocation, FRotator InRotation);
};
