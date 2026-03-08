// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BattleBaseUnit.h"
#include "BattleEnemyUnit.generated.h"

class UUnitDataAsset;

// 재화 드롭 정보를 담는 구조체
USTRUCT(BlueprintType)
struct FDropInfo
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ItemName;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MinAmount;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxAmount;
};

UCLASS()
class PCUBE_API ABattleEnemyUnit : public ABattleBaseUnit
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, Category="Drop", meta=(DeprecatedProperty, DeprecationMessage="Use SpawnDataAsset loot settings instead."))
	TArray<FDropInfo> DropTable;
	
	virtual void Die() override;
};
