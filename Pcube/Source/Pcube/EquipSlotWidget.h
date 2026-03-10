// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EquipSlotWidget.generated.h"

// ---
// EquipSlotWidget은 장비 일러스트 위에 배치되는 부품 슬롯 버튼
// ---

class UButton;
class UImage;
class UTextBlock;
class UWeaponPartDataAsset;

// 장착 슬롯의 식별자는 SocketName(FName)으로 단일화한다.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEquipSlotSelected, FName, SlotSocketName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEquipSlotUnequip, FName, SlotSocketName);

UCLASS()
class PCUBE_API UEquipSlotWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void InitSlot(FName InPartSlotKey, FText InDisplayName);
	void SetEquipped(UWeaponPartDataAsset* Part);

	UPROPERTY(BlueprintAssignable)
	FOnEquipSlotSelected OnSelected;

	UPROPERTY(BlueprintAssignable)
	FOnEquipSlotUnequip OnUnequip;

protected:
	virtual void NativeConstruct() override;

private:
	UFUNCTION()
	void HandleSelectClicked();

	UFUNCTION()
	void HandleUnequipClicked();

private:
	UPROPERTY(meta=(BindWidget))
	UButton* Btn_Select = nullptr;

	UPROPERTY(meta=(BindWidgetOptional))
	UButton* Btn_Unequip = nullptr;

	UPROPERTY(meta=(BindWidget))
	UImage* Img_Icon = nullptr;

	UPROPERTY(meta=(BindWidgetOptional))
	UTextBlock* Text_SlotName = nullptr;

private:
	FName PartSlotKey = NAME_None;

	UPROPERTY()
	TObjectPtr<UWeaponPartDataAsset> EquippedPart = nullptr;
};
