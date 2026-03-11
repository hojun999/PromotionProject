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
	const bool bAvailableVal = (Part != nullptr && Quantity > 0);

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
		Img_Icon->SetRenderOpacity(bAvailableVal ? 1.f : 0.35f);
	}

	if (Text_Name)
	{
		Text_Name->SetText(Part ? Part->DisplayName : FText::GetEmpty());
	}

	if (Text_Count)
	{
		// 수량 0이면 "없음", 있으면 수량 표시
		const FString CountStr = bAvailableVal
			? FString::Printf(TEXT("x%d"), Quantity)
			: TEXT("없음");
		Text_Count->SetText(FText::FromString(CountStr));
	}

	if (Btn_Root)
	{
		Btn_Root->SetIsEnabled(bAvailableVal);
		UE_LOG(LogTemp, Warning, TEXT("[SlotWidget] Init Part=%s Qty=%d bAvailable=%d IsEnabled=%d"),
	*GetNameSafe(Part), Quantity, bAvailableVal ? 1 : 0, Btn_Root->GetIsEnabled() ? 1 : 0);
	}
	
}

void UEquipPartsSlotWidget::HandleClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("[SlotWidget] HandleClicked Part=%s"), *GetNameSafe(Part));
	if (Part)
	{
		OnClicked.Broadcast(Part);
	}
}
