// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventorySlotWidget.generated.h"

class UButton;
class UImage;
class UTextBlock;
class UItemDataAsset;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInventorySlotClicked, int32, SlotIndex, UItemDataAsset*, Item);

UCLASS()
class PCUBE_API UInventorySlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void InitFilled(UItemDataAsset* InItem, int32 InQuantity, int32 InSlotIndex);
	void InitEmpty(int32 InSlotIndex);

	UPROPERTY(BlueprintAssignable)
	FOnInventorySlotClicked OnInventorySlotClicked;

protected:
	virtual void NativeConstruct() override;

private:
	UFUNCTION()
	void HandleClicked();

private:
	UPROPERTY(meta=(BindWidget))
	UButton* Btn_Select = nullptr;

	UPROPERTY(meta=(BindWidget))
	UImage* Img_Icon = nullptr;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* Text_Count = nullptr;

private:
	UPROPERTY()
	TObjectPtr<UItemDataAsset> Item = nullptr;

	int32 Quantity = 0;
	int32 SlotIndex = -1;
	bool bHasItem = false;
};
