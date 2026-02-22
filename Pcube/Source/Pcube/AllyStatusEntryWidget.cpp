// Fill out your copyright notice in the Description page of Project Settings.


#include "AllyStatusEntryWidget.h"
#include "UnitDataAsset.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UAllyStatusEntryWidget::InitWithUnit(ABattleBaseUnit* InUnit)
{
	Unit = InUnit;
	if (!IsValid(Unit)) return;
	
	// 초상화/이름
	if (Unit->UnitData)
	{
		if (PortraitImage && Unit->UnitData->PortraitTexture)
		{
			PortraitImage->SetBrushFromTexture(Unit->UnitData->PortraitTexture);
		}
		
		// 필요하면 유닛 이름 할당 추가
	}
	
	// 델리게이트 바인딩(중복 방지)
	Unit->OnHPChanged.RemoveDynamic(this, &UAllyStatusEntryWidget::HandleHPChanged);
	Unit->OnHPChanged.AddDynamic(this, &UAllyStatusEntryWidget::HandleHPChanged);
	
	Unit->OnUnitDied.RemoveDynamic(this, &UAllyStatusEntryWidget::HandleDied);
	Unit->OnUnitDied.AddDynamic(this, &UAllyStatusEntryWidget::HandleDied);
	
	// 초기 갱신
	HandleHPChanged(Unit->CurrentHP, Unit->GetMaxHP());
}

void UAllyStatusEntryWidget::HandleHPChanged(float CurrentHP, float MaxHP)
{
	const float Percent = (MaxHP > 0.f) ? (CurrentHP / MaxHP) : 0.f;
	if (HPBar) HPBar->SetPercent(Percent);
	
	if (HPText)
	{
		HPText->SetText(FText::FromString(
			FString::Printf(TEXT("%.0f / %.0f"), CurrentHP, MaxHP)
		));
	}
}

void UAllyStatusEntryWidget::HandleDied()
{
	// 회색 처리
	SetRenderOpacity(0.35f);
}

void UAllyStatusEntryWidget::SetActive(bool bActive)
{
	if (ActiveBorder)
	{
		ActiveBorder->SetVisibility(bActive ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
}
