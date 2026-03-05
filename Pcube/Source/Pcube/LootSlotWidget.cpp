// Fill out your copyright notice in the Description page of Project Settings.


#include "LootSlotWidget.h"

#include "ItemDataAsset.h"
#include "WeaponPartDataAsset.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void ULootSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (Btn_Select)
	{
		Btn_Select->OnClicked.RemoveDynamic(this, &ULootSlotWidget::HandleClicked);
		Btn_Select->OnClicked.AddDynamic(this, &ULootSlotWidget::HandleClicked);
	}
}

void ULootSlotWidget::InitFilled(const FLootStack& InStack, int32 InSlotIndex)
{
	SlotIndex = InSlotIndex;
	bHasItem = true;
	
	if (!InStack.Item)
	{
		InitEmpty(InSlotIndex);
		return;
	}
	
	if (Img_Icon && InStack.Item->Icon)
	{
		Img_Icon->SetVisibility(ESlateVisibility::Visible);
		Img_Icon->SetBrushFromTexture(InStack.Item->Icon);
	}
	else if (Img_Icon)
	{
		Img_Icon->SetVisibility(ESlateVisibility::Hidden);
	}
	
	if (Text_Count)
	{
		Text_Count->SetText(FText::FromString(FString::Printf(TEXT("%d"), InStack.Count)));
		Text_Count->SetVisibility(InStack.Count > 1 ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
	
	// Hover 효과 텍스트(툴팁) - 무기부품이면 효과 정보 텍스트
	FText EffectToolTipText;
	
	if (const UWeaponPartDataAsset* Part = Cast<UWeaponPartDataAsset>(InStack.Item))
	{
		// "이름\n효과" 의 형태
		const FString Str = FString::Printf(TEXT("%s\n%s"),
			*Part->DisplayName.ToString(),
			*Part->BuildEffectText().ToString());
		EffectToolTipText = FText::FromString(Str);
	}
	else
	{
		EffectToolTipText = InStack.Item->DisplayName;
	}
	
	SetToolTipText(EffectToolTipText);
	
	if (Btn_Select) Btn_Select->SetIsEnabled(true);
}

void ULootSlotWidget::InitEmpty(int32 InSlotIndex)
{
	SlotIndex = InSlotIndex;
	bHasItem = false;
	
	if (Img_Icon) Img_Icon->SetVisibility(ESlateVisibility::Hidden);
	if (Text_Count) Text_Count->SetVisibility(ESlateVisibility::Hidden);
	
	SetToolTipText(FText::GetEmpty());
	if (Btn_Select) Btn_Select->SetIsEnabled(false);
}

void ULootSlotWidget::HandleClicked()
{
	if (!bHasItem) return;
	OnLootSlotClicked.Broadcast(SlotIndex);
}
