#include "EquipPartsSlotWidget.h"

#include "WeaponPartDataAsset.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void UEquipPartsSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Btn_Select)
	{
		Btn_Select->OnClicked.RemoveDynamic(this, &UEquipPartsSlotWidget::HandleSelectClicked);
		Btn_Select->OnClicked.AddDynamic(this, &UEquipPartsSlotWidget::HandleSelectClicked);
	}

	if (Btn_Unequip)
	{
		Btn_Unequip->OnClicked.RemoveDynamic(this, &UEquipPartsSlotWidget::HandleUnequipClicked);
		Btn_Unequip->OnClicked.AddDynamic(this, &UEquipPartsSlotWidget::HandleUnequipClicked);
	}
}

void UEquipPartsSlotWidget::InitSlot(FName InSlotSocketName, FText InDisplayName)
{
	SlotSocketName = InSlotSocketName;

	if (Text_SlotName)
	{
		Text_SlotName->SetText(InDisplayName.IsEmpty() ? FText::FromName(SlotSocketName) : InDisplayName);
	}

	RefreshVisuals();
	RefreshTooltip();
}

void UEquipPartsSlotWidget::SetEquipped(UWeaponPartDataAsset* InEquippedPart)
{
	EquippedPart = InEquippedPart;
	RefreshVisuals();
	RefreshTooltip();
}

void UEquipPartsSlotWidget::SetCompatibleParts(const TArray<UWeaponPartDataAsset*>& InCompatibleParts)
{
	CompatibleParts.Reset();
	for (UWeaponPartDataAsset* Part : InCompatibleParts)
	{
		if (Part)
		{
			CompatibleParts.Add(Part);
		}
	}

	RefreshVisuals();
	RefreshTooltip();
}

void UEquipPartsSlotWidget::HandleSelectClicked()
{
	if (CompatibleParts.Num() <= 0)
	{
		return;
	}

	int32 CurrentIndex = INDEX_NONE;
	if (EquippedPart)
	{
		CurrentIndex = CompatibleParts.IndexOfByKey(EquippedPart);
	}

	const int32 NextIndex = (CurrentIndex == INDEX_NONE)
		? 0
		: ((CurrentIndex + 1) % CompatibleParts.Num());

	if (CompatibleParts.IsValidIndex(NextIndex) && CompatibleParts[NextIndex])
	{
		OnEquipRequested.Broadcast(SlotSocketName, CompatibleParts[NextIndex]);
	}
}

void UEquipPartsSlotWidget::HandleUnequipClicked()
{
	if (!EquippedPart) return;
	OnUnequipRequested.Broadcast(SlotSocketName);
}

void UEquipPartsSlotWidget::RefreshVisuals()
{
	if (Img_Icon)
	{
		if (EquippedPart && EquippedPart->Icon)
		{
			Img_Icon->SetBrushFromTexture(EquippedPart->Icon);
			Img_Icon->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			Img_Icon->SetVisibility(ESlateVisibility::Hidden);
		}
	}

	if (Btn_Unequip)
	{
		Btn_Unequip->SetVisibility(EquippedPart ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}

	if (Text_CompatibleCount)
	{
		Text_CompatibleCount->SetText(FText::FromString(FString::Printf(TEXT("%d"), CompatibleParts.Num())));
	}

	if (Btn_Select)
	{
		Btn_Select->SetIsEnabled(CompatibleParts.Num() > 0 || EquippedPart != nullptr);
	}
}

void UEquipPartsSlotWidget::RefreshTooltip()
{
	TArray<FString> Lines;
	Lines.Add(FString::Printf(TEXT("Slot: %s"), *SlotSocketName.ToString()));

	if (EquippedPart)
	{
		Lines.Add(FString::Printf(TEXT("Equipped: %s"), *EquippedPart->DisplayName.ToString()));
		Lines.Add(EquippedPart->BuildEffectText().ToString());
	}
	else
	{
		Lines.Add(TEXT("Equipped: None"));
	}

	Lines.Add(FString::Printf(TEXT("Compatible in Inventory: %d"), CompatibleParts.Num()));

	if (CompatibleParts.Num() > 0)
	{
		Lines.Add(TEXT("Click slot to cycle compatible parts."));
	}
	else
	{
		Lines.Add(TEXT("No compatible part in inventory."));
	}

	SetToolTipText(FText::FromString(FString::Join(Lines, TEXT("\n"))));
}
