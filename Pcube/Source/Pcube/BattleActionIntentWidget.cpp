#include "BattleActionIntentWidget.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "ItemDataAsset.h"
#include "SkillDataAsset.h"

void UBattleActionIntentWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SetVisibility(ESlateVisibility::Collapsed);
}

void UBattleActionIntentWidget::ShowSkillIntent(USkillDataAsset* Skill)
{
	if (!Skill)
	{
		ClearIntent();
		return;
	}

	ApplyIntent(Skill->SkillImage, FText::FromString(Skill->SkillName));
}

void UBattleActionIntentWidget::ShowItemIntent(UItemDataAsset* Item)
{
	if (!Item)
	{
		ClearIntent();
		return;
	}

	ApplyIntent(Item->Icon.Get(), Item->DisplayName);
}

void UBattleActionIntentWidget::ClearIntent()
{
	SetVisibility(ESlateVisibility::Collapsed);
}

void UBattleActionIntentWidget::ApplyIntent(UTexture2D* Icon, const FText& Label)
{
	if (Img_Action)
	{
		if (Icon)
		{
			Img_Action->SetBrushFromTexture(Icon, true);
		}
		else
		{
			FSlateBrush EmptyBrush;
			Img_Action->SetBrush(EmptyBrush);
		}
	}

	if (Text_ActionName)
	{
		Text_ActionName->SetText(Label);
	}

	SetVisibility(ESlateVisibility::Visible);
}
