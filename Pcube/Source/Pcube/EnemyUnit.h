// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseUnit.h"
#include "EnemyUnit.generated.h"

/**
 * 
 */

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
class PCUBE_API AEnemyUnit : public ABaseUnit
{
	GENERATED_BODY()
	
public:
	AEnemyUnit();
	
	UPROPERTY(EditAnywhere, Category="Drop")
	TArray<FDropInfo> DropTable;
	
	virtual void Die() override;
};
