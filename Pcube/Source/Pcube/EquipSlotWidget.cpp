// Fill out your copyright notice in the Description page of Project Settings.


#include "EquipSlotWidget.h"
#include "WeaponPartDataAsset.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void UEquipSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Btn_Select)
	{
		Btn_Select->OnClicked.RemoveDynamic(this, &UEquipSlotWidget::HandleSelectClicked);
		Btn_Select->OnClicked.AddDynamic(this, &UEquipSlotWidget::HandleSelectClicked);
	}
	if (Btn_Unequip)
	{
		Btn_Unequip->OnClicked.RemoveDynamic(this, &UEquipSlotWidget::HandleUnequipClicked);
		Btn_Unequip->OnClicked.AddDynamic(this, &UEquipSlotWidget::HandleUnequipClicked);
	}
}

void UEquipSlotWidget::InitSlot(FName InSlotSocketName, FText InDisplayName)
{
	SlotSocketName = InSlotSocketName;

	if (Text_SlotName)
	{
		Text_SlotName->SetText(InDisplayName.IsEmpty() ? FText::FromName(SlotSocketName) : InDisplayName);
	}

	// if (Text_Slot)
	// {
	// 	Text_Slot->SetText(FText::FromName(SlotSocketName));
	// }
	SetEquipped(nullptr);
}

void UEquipSlotWidget::SetEquipped(UWeaponPartDataAsset* Part)
{
	EquippedPart = Part;

	if (EquippedPart && EquippedPart->Icon && Img_Icon)
	{
		Img_Icon->SetVisibility(ESlateVisibility::Visible);
		Img_Icon->SetBrushFromTexture(EquippedPart->Icon);
	}
	else if (Img_Icon)
	{
		Img_Icon->SetVisibility(ESlateVisibility::Hidden);
	}

	if (Btn_Unequip)
	{
		Btn_Unequip->SetVisibility(EquippedPart ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}

	// 툴팁
	if (EquippedPart)
	{
		const FString Str = FString::Printf(TEXT("%s\n%s"),
			*EquippedPart->DisplayName.ToString(),
			*EquippedPart->BuildEffectText().ToString());
		SetToolTipText(FText::FromString(Str));
	}
	else
	{
		SetToolTipText(FText::GetEmpty());
	}
}

void UEquipSlotWidget::HandleSelectClicked()
{
	OnSelected.Broadcast(SlotSocketName);
}

void UEquipSlotWidget::HandleUnequipClicked()
{
	OnUnequip.Broadcast(SlotSocketName);
}