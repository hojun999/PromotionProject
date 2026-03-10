#include "EquipPartsSlotWidget.h"

#include "WeaponPartDataAsset.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void UEquipPartsSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Btn_Root)
	{
		Btn_Root->OnClicked.RemoveDynamic(this, &UEquipPartsSlotWidget::HandleClicked);
		Btn_Root->OnClicked.AddDynamic(this, &UEquipPartsSlotWidget::HandleClicked);
	}
}

void UEquipPartsSlotWidget::Init(UWeaponPartDataAsset* InPart, int32 InQuantity)
{
	Part = InPart;
	Quantity = InQuantity;

	if (Img_Icon)
	{
		if (Part && Part->Icon)
		{
			Img_Icon->SetBrushFromTexture(Part->Icon);
			Img_Icon->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			Img_Icon->SetVisibility(ESlateVisibility::Hidden);
		}
	}

	if (Text_Name)
	{
		Text_Name->SetText(Part ? Part->DisplayName : FText::GetEmpty());
	}

	if (Text_Count)
	{
		Text_Count->SetText(FText::FromString(FString::Printf(TEXT("x%d"), FMath::Max(1, Quantity))));
	}

	if (Btn_Root)
	{
		Btn_Root->SetIsEnabled(Part != nullptr);
	}
}

void UEquipPartsSlotWidget::HandleClicked()
{
	if (Part)
	{
		OnClicked.Broadcast(Part);
	}
}
