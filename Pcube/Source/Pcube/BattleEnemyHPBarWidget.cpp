#include "BattleEnemyHPBarWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UBattleEnemyHPBarWidget::InitForUnitName(const FString& InName)
{
	if (NameText)
	{
		NameText->SetText(FText::FromString(InName));
	}
}

void UBattleEnemyHPBarWidget::SetHP(float CurrentHP, float MaxHP)
{
	if (HPBar)
	{
		const float Percent = (MaxHP > KINDA_SMALL_NUMBER) ? FMath::Clamp(CurrentHP / MaxHP, 0.f, 1.f) : 0.f;
		HPBar->SetPercent(Percent);
	}
}
