#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UnitDataAsset.h"
#include "EquipPartsWindowWidget.generated.h"

class UButton;
class UTextBlock;
class UUniformGridPanel;
class UEquipPartsSlotWidget;
class UEquipmentSubsystem;
class UInventorySubsystem;
class UBattleInfoTransferSubsystem;
class UWeaponDataAsset;
class UWeaponPartDataAsset;

UCLASS()
class PCUBE_API UEquipPartsWindowWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	bool OpenForEquipment(int32 InPartyIndex, FName InEquipmentKey);
	void CloseWindow();
	bool IsOpenFor(int32 InPartyIndex, FName InEquipmentKey) const;
	void SetWindowScreenPosition(FVector2D InViewportPos);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	UFUNCTION() void HandleCloseClicked();
	UFUNCTION() void HandleEquipmentChanged(int32 ChangedPartyIndex);
	UFUNCTION() void HandleSlotEquipRequested(FName SlotSocketName, UWeaponPartDataAsset* PartToEquip);
	UFUNCTION() void HandleSlotUnequipRequested(FName SlotSocketName);

	void RefreshWindow();
	bool ResolveEquipmentButtonDef(FPlayerInfoEquipmentButtonDef& OutDef) const;
	UUnitDataAsset* GetPartyUnitData(int32 PartyIndex) const;
	UWeaponDataAsset* ResolveTargetWeapon(const FPlayerInfoEquipmentButtonDef& Def) const;
	void GatherCompatibleParts(UWeaponDataAsset* Weapon, FName SlotSocketName, TArray<UWeaponPartDataAsset*>& OutParts) const;

private:
	UPROPERTY(meta=(BindWidgetOptional))
	UUniformGridPanel* Grid_Slots = nullptr;

	UPROPERTY(meta=(BindWidgetOptional))
	UButton* Btn_Close = nullptr;

	UPROPERTY(meta=(BindWidgetOptional))
	UTextBlock* Text_Title = nullptr;

	UPROPERTY(EditAnywhere, Category="EquipParts|UI")
	TSubclassOf<UEquipPartsSlotWidget> SlotWidgetClass;

	UPROPERTY()
	TObjectPtr<UEquipmentSubsystem> EquipmentSubsystem = nullptr;

	UPROPERTY()
	TObjectPtr<UInventorySubsystem> InventorySubsystem = nullptr;

	UPROPERTY()
	TObjectPtr<UBattleInfoTransferSubsystem> TransferSubsystem = nullptr;

	int32 OpenPartyIndex = INDEX_NONE;
	FName OpenEquipmentKey = NAME_None;

	UPROPERTY()
	TObjectPtr<UWeaponDataAsset> ResolvedWeapon = nullptr;
};
