// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "BattleTurnManager.generated.h"

// 각 유닛의 해당 라운드 턴 정보를 담는 구조체
USTRUCT(BlueprintType)
struct FBattleTurnUnit
{
	GENERATED_BODY()
	
	UPROPERTY()
	AActor* UnitActor = nullptr;
	
	UPROPERTY()
	int32 InitialSpeedValue = 0; // BaseSpeed + Random Roll (0, 8) -> 다키스트 던전의 턴제 시스템
	
	UPROPERTY()
	bool bHasActed = false; // 이번 라운드 행동 완료 여부
};

UCLASS()
class PCUBE_API UBattleTurnManager : public UObject
{
	GENERATED_BODY()
	
public:
	// 매 라운드 시작 시 호출: 주사위 롤 & 순서 정렬
	UFUNCTION(BlueprintCallable, Category="Battle|Turn")
	void InitNewRound(const TArray<AActor*>& ParticipatingUnits);
	
	// 다음 행동할 유닛 반환. 없을 경우 nullptr 반환(= 라운드 종료)
	UFUNCTION(BlueprintCallable, Category="Battle|Turn")
	AActor* GetNextUnit();
	
	// 모든 유닛의 행동 종료 확인
	UFUNCTION(BlueprintCallable, Category="Battle|Turn")
	bool IsRoundFinished() const;
	
	// 현재 라운드 진행 인덱스
	UPROPERTY(BlueprintReadOnly, Category="Battle|Turn")
	int32 CurrentRound = 0;
	
	
private:
	UPROPERTY()
	TArray<FBattleTurnUnit> RoundArray;
	
	// 유닛에서 속도 값을 가져오는 헬퍼 함수
	// TODO: AI는 해당 함수를 추후 인터페이스로 확장 권장
	float GetUnitSpeed(AActor* UnitActor) const;
	
};
