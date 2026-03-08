// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ItemDataAsset.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "InventorySubsystem.generated.h"

USTRUCT(BlueprintType)
struct FInventoryStack
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UItemDataAsset> Item = nullptr; // 슬롯에 든 아이템
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 Quantity = 0; // 수량
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryChanged); // 인벤토리 변경 시 UI 갱신 트리거

UCLASS()
class PCUBE_API UInventorySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintAssignable, Category="Inventory")
	FOnInventoryChanged OnInventoryChanged; // UI 갱신 이벤트
	
	UFUNCTION(BlueprintCallable, Category="Inventory")
	int32 AddItem(UItemDataAsset* Item, int32 Amount); // 실제로 추가된 수량 반환
	
	UFUNCTION(BlueprintCallable, Category="Inventory")
	bool RemoveItem(UItemDataAsset* Item, int32 Amount); // 아이템 제거 - 수량 부족이면 실패
	
	UFUNCTION(BlueprintCallable, Category="Inventory")
	int32 GetQuantity(UItemDataAsset* Item) const; // 특정 아이템 보유 수량 조회

	UFUNCTION(BlueprintCallable, Category="Inventory")
	int32 GetFreeSlotCount() const; // 장비/루팅 안전성 체크용
	
	UFUNCTION(BlueprintCallable, Category="Inventory")
	const TArray<FInventoryStack>& GetAllStacks() const { return Stacks; } // UI 표시용
	
private:
	UPROPERTY()
	int32 MaxSlots = 64; // 인벤토리 최대 슬롯 수
	
	UPROPERTY()
	TArray<FInventoryStack> Stacks; // 인벤토리 슬롯 배열 (고정 크기)
};
