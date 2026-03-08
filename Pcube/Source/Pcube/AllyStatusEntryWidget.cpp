// Fill out your copyright notice in the Description page of Project Settings.


#include "AllyStatusEntryWidget.h"
#include "UnitDataAsset.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UAllyStatusEntryWidget::NativeDestruct()
{
	Super::NativeDestruct();
	
	if (IsValid(Unit))
	{
		Unit->OnHPChanged.RemoveDynamic(this, &UAllyStatusEntryWidget::HandleHPChanged);
		Unit->OnUnitDied.RemoveDynamic(this, &UAllyStatusEntryWidget::HandleDied);
		Unit->OnSkillPointsChanged.RemoveDynamic(this, &UAllyStatusEntryWidget::HandleSkillPointsChanged);
	}
}

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
	
	// HP 델리게이트 바인딩(중복 방지)
	Unit->OnHPChanged.RemoveDynamic(this, &UAllyStatusEntryWidget::HandleHPChanged);
	Unit->OnHPChanged.AddDynamic(this, &UAllyStatusEntryWidget::HandleHPChanged);
	
	Unit->OnUnitDied.RemoveDynamic(this, &UAllyStatusEntryWidget::HandleDied);
	Unit->OnUnitDied.AddDynamic(this, &UAllyStatusEntryWidget::HandleDied);
	
	// SP 델리게이트 바인딩
	Unit->OnSkillPointsChanged.RemoveDynamic(this, &UAllyStatusEntryWidget::HandleSkillPointsChanged);
	Unit->OnSkillPointsChanged.AddDynamic(this, &UAllyStatusEntryWidget::HandleSkillPointsChanged);
	
	// HP 초기 갱신
	HandleHPChanged(Unit->CurrentHP, Unit->GetMaxHP());
	
	// SP 초기 갱신
	HandleSkillPointsChanged(Unit->GetCurrentSkillPoints(), Unit->GetMaxSkillPoints());
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

void UAllyStatusEntryWidget::HandleSkillPointsChanged(int32 Current, int32 Max)
{
	if (!HB_SkillPoints)
	{
		return; // HorizontalBox가 없으면 진행 불가 - 지정 필요
	}
	
	// SP 시스템이 없는 유닛이면 숨김
	if (Max <= 0)
	{
		HB_SkillPoints->SetVisibility(ESlateVisibility::Collapsed);
		HB_SkillPoints->ClearChildren();
		SkillPointIcons.Empty();
		CachedMaxSP = 0;
		return;
	}
	
	HB_SkillPoints->SetVisibility(ESlateVisibility::Visible);
	
	// Max가 바뀌면 아이콘 수를 다시 생성
	if (CachedMaxSP != Max)
	{
		RebuildSkillPointsIcons(Max);
		CachedMaxSP = Max;
	}
	
	// Current 기준으로 앞에서부터 채움
	UpdateSkillPointFill(Current);
}

void UAllyStatusEntryWidget::RebuildSkillPointsIcons(int32 Max)
{
	if (!HB_SkillPoints) return;
	
	HB_SkillPoints->ClearChildren();
	SkillPointIcons.Empty();
	SkillPointIcons.Reserve(Max);
	
	// 텍스처가 없으면 표시 불가 - 할당 필요
	if (!SkillPointIconTexture)
	{
		return;
	}
	
	for (int32 i = 0; i < Max; ++i)
	{
		UImage* Img = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass()); // SP 아이콘 1칸 생성
		if (!Img) continue;
		
		FSlateBrush Brush;
		Brush.SetResourceObject(SkillPointIconTexture);
		Img->SetBrush(Brush);
		
		Img->SetColorAndOpacity(SkillPointEmptyColor); // 기본은 비어있는 색
		if (UHorizontalBoxSlot* HBoxSlot  = HB_SkillPoints->AddChildToHorizontalBox(Img))
		{
			HBoxSlot->SetVerticalAlignment(VAlign_Center);
		}
		
		SkillPointIcons.Add(Img);
	}
}

void UAllyStatusEntryWidget::UpdateSkillPointFill(int32 Current)
{
	const int32 Max = SkillPointIcons.Num();
	if (Max <= 0) return;
	
	const int32 Clamped = FMath::Clamp(Current, 0, Max);
	
	for (int32 i = 0; i < Max; ++i)
	{
		if (UImage* Img = SkillPointIcons[i])
		{
			Img->SetColorAndOpacity(i < Clamped ? SkillPointFilledColor : SkillPointEmptyColor); // 앞부터 Current 만큼 채움
		}
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
