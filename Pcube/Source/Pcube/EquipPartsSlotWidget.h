#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EquipPartsSlotWidget.generated.h"

class UButton;
class UImage;
class UTextBlock;
class UWeaponPartDataAsset;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEquipPartsSlotEquipRequested, FName, SlotSocketName, UWeaponPartDataAsset*, PartToEquip);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEquipPartsSlotUnequipRequested, FName, SlotSocketName);

UCLASS()
class PCUBE_API UEquipPartsSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void InitSlot(FName InSlotSocketName, FText InDisplayName);
	void SetEquipped(UWeaponPartDataAsset* InEquippedPart);
	void SetCompatibleParts(const TArray<UWeaponPartDataAsset*>& InCompatibleParts);

	UPROPERTY(BlueprintAssignable)
	FOnEquipPartsSlotEquipRequested OnEquipRequested;

	UPROPERTY(BlueprintAssignable)
	FOnEquipPartsSlotUnequipRequested OnUnequipRequested;

protected:
	virtual void NativeConstruct() override;

private:
	UFUNCTION() void HandleSelectClicked();
	UFUNCTION() void HandleUnequipClicked();

	void RefreshVisuals();
	void RefreshTooltip();

private:
	UPROPERTY(meta=(BindWidget))
	UButton* Btn_Select = nullptr;

	UPROPERTY(meta=(BindWidgetOptional))
	UButton* Btn_Unequip = nullptr;

	UPROPERTY(meta=(BindWidgetOptional))
	UImage* Img_Icon = nullptr;

	UPROPERTY(meta=(BindWidgetOptional))
	UTextBlock* Text_SlotName = nullptr;

	UPROPERTY(meta=(BindWidgetOptional))
	UTextBlock* Text_CompatibleCount = nullptr;

	FName SlotSocketName = NAME_None;

	UPROPERTY()
	TObjectPtr<UWeaponPartDataAsset> EquippedPart = nullptr;

	UPROPERTY()
	TArray<TObjectPtr<UWeaponPartDataAsset>> CompatibleParts;
};
