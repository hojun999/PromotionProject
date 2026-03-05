#include "WeaponModSlotWidget.h"
#include "WeaponPartDataAsset.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void UWeaponModSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Btn_Select)
	{
		Btn_Select->OnClicked.RemoveDynamic(this, &UWeaponModSlotWidget::HandleSelect);
		Btn_Select->OnClicked.AddDynamic(this, &UWeaponModSlotWidget::HandleSelect);
	}

	if (Btn_Unequip)
	{
		Btn_Unequip->OnClicked.RemoveDynamic(this, &UWeaponModSlotWidget::HandleUnequip);
		Btn_Unequip->OnClicked.AddDynamic(this, &UWeaponModSlotWidget::HandleUnequip);
	}
}

void UWeaponModSlotWidget::Init(FName InSlotId, FText InDisplayName)
{
	SlotId = InSlotId;

	if (Text_Name)
	{
		Text_Name->SetText(InDisplayName);
	}

	SetEquipped(nullptr);
}

void UWeaponModSlotWidget::SetEquipped(UWeaponPartDataAsset* Part)
{
	Equipped = Part;

	// 아이콘
	if (Img_Icon)
	{
		if (Equipped && Equipped->Icon)
		{
			Img_Icon->SetVisibility(ESlateVisibility::Visible);
			Img_Icon->SetBrushFromTexture(Equipped->Icon);
		}
		else
		{
			Img_Icon->SetVisibility(ESlateVisibility::Hidden);
		}
	}

	// 해제 버튼
	if (Btn_Unequip)
	{
		const bool bHasPart = (Equipped != nullptr);
		Btn_Unequip->SetVisibility(bHasPart ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		Btn_Unequip->SetIsEnabled(bHasPart);
	}

	// 툴팁
	if (Equipped)
	{
		const FString Tip = FString::Printf(
			TEXT("%s\n%s"),
			*Equipped->DisplayName.ToString(),
			*Equipped->BuildEffectText().ToString()
		);
		SetToolTipText(FText::FromString(Tip));
	}
	else
	{
		SetToolTipText(FText::GetEmpty());
	}
}

void UWeaponModSlotWidget::HandleSelect()
{
	OnSelected.Broadcast(SlotId);
}

void UWeaponModSlotWidget::HandleUnequip()
{
	if (!Equipped) return;
	OnUnequip.Broadcast(SlotId);
}