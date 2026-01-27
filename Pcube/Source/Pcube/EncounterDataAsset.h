// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "EncounterDataAsset.generated.h"

class UUnitDataAsset;

USTRUCT(BlueprintType)
struct FEncounterUnit
{
	GENERATED_BODY()
	
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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Battle")
	FString PositionName;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Battle")
	UUnitDataAsset* UnitData;
};

UCLASS()
class PCUBE_API UEncounterDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
};
