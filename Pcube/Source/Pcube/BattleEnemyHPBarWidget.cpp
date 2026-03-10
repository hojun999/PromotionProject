#include "BattleEnemyHPBarWidget.h"

#include "Components/Border.h"
#include "Components/HorizontalBox.h"
#include "Components/Overlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Blueprint/WidgetTree.h"

void UBattleEnemyHPBarWidget::EnsureWidgetTreeBuilt()
{
	if (!WidgetTree) return;
	if (WidgetTree->RootWidget) return;

	UOverlay* Root = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("Root"));
	WidgetTree->RootWidget = Root;

	UBorder* Frame = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Frame"));
	Frame->SetPadding(FMargin(4.f, 2.f));
	Root->AddChild(Frame);

	UHorizontalBox* Box = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("Box"));
	Frame->SetContent(Box);

	HPBar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), TEXT("EnemyHPBar"));
	HPBar->SetPercent(1.f);
	Box->AddChildToHorizontalBox(HPBar);

	NameText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("EnemyName"));
	NameText->SetVisibility(ESlateVisibility::Collapsed);
	Root->AddChild(NameText);
}

void UBattleEnemyHPBarWidget::NativeConstruct()
{
	Super::NativeConstruct();
	EnsureWidgetTreeBuilt();
}

void UBattleEnemyHPBarWidget::InitForUnitName(const FString& InName)
{
	EnsureWidgetTreeBuilt();
	if (NameText)
	{
		NameText->SetText(FText::FromString(InName));
	}
}

void UBattleEnemyHPBarWidget::SetHP(float CurrentHP, float MaxHP)
{
	EnsureWidgetTreeBuilt();
	if (HPBar)
	{
		const float Percent = (MaxHP > KINDA_SMALL_NUMBER) ? FMath::Clamp(CurrentHP / MaxHP, 0.f, 1.f) : 0.f;
		HPBar->SetPercent(Percent);
	}
}
