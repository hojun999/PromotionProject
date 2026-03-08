// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerInfoEquipButtonWidget.h"

#include "Components/Button.h"
#include "Components/Image.h"

void UPlayerInfoEquipButtonWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Btn_Root)
	{
		Btn_Root->OnClicked.RemoveDynamic(this, &UPlayerInfoEquipButtonWidget::HandleClicked);
		Btn_Root->OnClicked.AddDynamic(this, &UPlayerInfoEquipButtonWidget::HandleClicked);
	}
}

void UPlayerInfoEquipButtonWidget::Init(FName InEquipmentKey, UTexture2D* InIcon)
{
	EquipmentKey = InEquipmentKey;

	if (Img_Icon)
	{
		if (InIcon)
		{
			Img_Icon->SetBrushFromTexture(InIcon);
			Img_Icon->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			// Icon can be intentionally empty (future expansion / placeholder)
			Img_Icon->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void UPlayerInfoEquipButtonWidget::HandleClicked()
{
	OnClicked.Broadcast(EquipmentKey);
}
