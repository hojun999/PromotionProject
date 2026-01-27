// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "BattleGameMode.generated.h"

// ???: 아래 구조체처럼의 구분이 필요한가..?
UENUM(BlueprintType)
enum class EBattleState : uint8
{
	Waiting,	// 오브젝트&데이터 로드 중
	Spawning,	// 유닛 배치 중
	AllyTurn,	// 아군 턴
	EnemyTurn,	// 적군 턴
	BattleEnd	// 전투 종료
};

/**
 * 
 */
UCLASS()
class PCUBE_API ABattleGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	virtual void BeginPlay() override;
	
	// 현재 전투 상태
	UPROPERTY(BlueprintReadOnly, Category="Battle")
	EBattleState CurrentState;
	
	UPROPERTY(EditAnywhere, Category="Battle")
	TSubclassOf<AActor> BattleManager;
};
