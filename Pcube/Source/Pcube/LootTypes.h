#pragma once

#include "CoreMinimal.h"
#include "LootTypes.generated.h"

class UItemDataAsset;

USTRUCT(BlueprintType)
struct FLootStack
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UItemDataAsset> Item = nullptr; // 루팅 아이템(무기부품/소모품)

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Count = 1; // 수량
};

USTRUCT(BlueprintType)
struct FEncounterLootList
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FLootStack> Items; // 해당 Encounter의 잔여 루팅 아이템 목록
};

USTRUCT(BlueprintType)
struct FLootDropEntry
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UItemDataAsset> Item = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0.0"))
	float Weight = 1.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="1"))
	int32 MinCount = 1;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="1"))
	int32 MaxCount = 1;
};
