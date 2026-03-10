#pragma once

#include "CoreMinimal.h"
#include "UnitStatTypes.generated.h"

USTRUCT(BlueprintType)
struct FUnitBaseStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats")
	float MaxHP = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats")
	float Speed = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats")
	float AttackPower = 10.f;
};

USTRUCT(BlueprintType)
struct FEquipmentStatBonus
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats")
	float MaxHP = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats")
	float Speed = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats")
	float AttackPower = 0.f;
};
