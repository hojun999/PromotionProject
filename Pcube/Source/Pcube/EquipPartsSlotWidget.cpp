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
	bAvailable = (InQuantity > 0);

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
		// 보유 중이면 수량 표시, 없으면 "없음" 표시
		if (bAvailable)
		{
			Text_Count->SetText(FText::FromString(FString::Printf(TEXT("x%d"), Quantity)));
		}
		else
		{
			Text_Count->SetText(FText::FromString(TEXT("없음")));
		}
	}

	ApplyAvailability(bAvailable);
}

void UEquipPartsSlotWidget::ApplyAvailability(bool bIsAvailable)
{
	// 버튼: 인벤에 없으면 클릭 불가
	if (Btn_Root)
	{
		Btn_Root->SetIsEnabled(bIsAvailable && Part != nullptr);
	}

	// 아이콘: 인벤에 없으면 어둡게
	if (Img_Icon)
	{
		Img_Icon->SetRenderOpacity(bIsAvailable ? 1.f : UnavailableOpacity);
	}

	// 이름 텍스트도 같이 어둡게
	if (Text_Name)
	{
		Text_Name->SetRenderOpacity(bIsAvailable ? 1.f : UnavailableOpacity);
	}
}

void UEquipPartsSlotWidget::HandleClicked()
{
	// bAvailable 체크는 버튼 비활성화로 이미 막혀있지만 방어적으로 한 번 더 체크
	if (Part && bAvailable)
	{
		OnClicked.Broadcast(Part);
	}
}
