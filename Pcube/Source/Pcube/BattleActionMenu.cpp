// Fill out your copyright notice in the Description page of Project Settings.


#include "BattleActionMenu.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/VerticalBox.h"

void UBattleActionMenu::ShowMenu(ABattleAllyUnit* TargetUnit)
{
	if (!TargetUnit) return;
	CurrentUnit = TargetUnit;
	
	UpdateMenuPosition();
	
	// 카메라가 이동하는 동안 매 프레임 위치 갱신
	bIsFollowingUnit = true;
	SetVisibility(ESlateVisibility::Visible);
	
	// 서브 메뉴들은 일단 숨김
	SkillListWidget->SetVisibility(ESlateVisibility::Collapsed);
	ItemListWidget->SetVisibility(ESlateVisibility::Collapsed);
}

void UBattleActionMenu::UpdateMenuPosition()
{
	if (!CurrentUnit || !CurrentUnit->UIAnchorPoint) return;
	
	APlayerController* PC = GetOwningPlayer();
	if (!PC) return;
	
	FVector2D ScreenPosition;
	FVector WorldLocation = CurrentUnit->UIAnchorPoint->GetComponentLocation();
	
	// 컴포넌트의 월드 좌표를 가져와서 투영
	if (UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(PC, WorldLocation, ScreenPosition, true))
	{
		SetPositionInViewport(ScreenPosition);
	}
	
}

void UBattleActionMenu::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	
	if (bIsFollowingUnit)
	{
		UpdateMenuPosition();
	}
}

void UBattleActionMenu::OnAttackClicked()
{
	
	UE_LOG(LogTemp, Warning, TEXT("[ActionMenu] Attack clicked."));
	
	OnActionRequested.Broadcast(BasicAttackData);
}

void UBattleActionMenu::OnSkillMenuClicked()
{
	OnSkillMenuRequested.Broadcast();
}

void UBattleActionMenu::OnItemMenuClicked()
{
	OnItemMenuRequested.Broadcast();
}
