// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ItemDataAsset.generated.h"

UENUM(BlueprintType)
enum class EItemType : uint8
{
	WeaponPart UMETA(DisplayName="WeaponPart"),
	Consumable UMETA(DisplayName="Consumable")
};

UCLASS(BlueprintType)
class PCUBE_API UItemDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item")
	FName ItemID = NAME_None; // 아이템 식별용 ID
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item")
	FText DisplayName; // UI 표시용 아이템 이름 텍스트
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item")
	TObjectPtr<UTexture2D> Icon = nullptr;	// UI 아이콘
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item")
	EItemType ItemType = EItemType::WeaponPart;	// 아이템 종류
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item")
	bool bStackable = false; // 아이템 스택 가능 여부
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item", meta=(EditCondition="bStackable", ClampMin="1"))
	int32 MaxStack = 1; // 스택 최대치
};
