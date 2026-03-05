// Fill out your copyright notice in the Description page of Project Settings.


#include "InventorySlotWidget.h"
#include "ItemDataAsset.h"
#include "WeaponPartDataAsset.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void UInventorySlotWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Btn_Select)
	{
		Btn_Select->OnClicked.RemoveDynamic(this, &UInventorySlotWidget::HandleClicked);
		Btn_Select->OnClicked.AddDynamic(this, &UInventorySlotWidget::HandleClicked);
	}
}

void UInventorySlotWidget::InitFilled(UItemDataAsset* InItem, int32 InQuantity, int32 InSlotIndex)
{
	Item = InItem;
	Quantity = InQuantity;
	SlotIndex = InSlotIndex;
	bHasItem = (Item != nullptr && Quantity > 0);

	if (!bHasItem)
	{
		InitEmpty(InSlotIndex);
		return;
	}

	if (Img_Icon && Item->Icon)
	{
		Img_Icon->SetVisibility(ESlateVisibility::Visible);
		Img_Icon->SetBrushFromTexture(Item->Icon);
	}
	else if (Img_Icon)
	{
		Img_Icon->SetVisibility(ESlateVisibility::Hidden);
	}

	if (Text_Count)
	{
		Text_Count->SetText(FText::AsNumber(Quantity));
		Text_Count->SetVisibility(Quantity > 1 ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}

	// 툴팁
	FText Tip;
	if (const UWeaponPartDataAsset* Part = Cast<UWeaponPartDataAsset>(Item))
	{
		const FString Str = FString::Printf(TEXT("%s\n%s"),
			*Part->DisplayName.ToString(),
			*Part->BuildEffectText().ToString());
		Tip = FText::FromString(Str);
	}
	else
	{
		Tip = Item->DisplayName;
	}
	SetToolTipText(Tip);

	if (Btn_Select) Btn_Select->SetIsEnabled(true);
}

void UInventorySlotWidget::InitEmpty(int32 InSlotIndex)
{
	Item = nullptr;
	Quantity = 0;
	SlotIndex = InSlotIndex;
	bHasItem = false;

	if (Img_Icon) Img_Icon->SetVisibility(ESlateVisibility::Hidden);
	if (Text_Count) Text_Count->SetVisibility(ESlateVisibility::Hidden);

	SetToolTipText(FText::GetEmpty());
	if (Btn_Select) Btn_Select->SetIsEnabled(false);
}

void UInventorySlotWidget::HandleClicked()
{
	if (!bHasItem) return;
	OnInventorySlotClicked.Broadcast(SlotIndex, Item.Get());
}