// Fill out your copyright notice in the Description page of Project Settings.


#include "BattleActionMenu.h"

#include "UnitDataAsset.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/Button.h"
#include "Components/VerticalBox.h"


void UBattleActionMenu::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (Btn_Attack)
	{
		Btn_Attack->OnClicked.RemoveDynamic(this, &UBattleActionMenu::OnAttackClicked);
		Btn_Attack->OnClicked.AddDynamic(this, &UBattleActionMenu::OnAttackClicked);
	}
	
	if (Btn_Skill)
	{
		Btn_Skill->OnClicked.RemoveDynamic(this, &UBattleActionMenu::OnSkillMenuClicked);
		Btn_Skill->OnClicked.AddDynamic(this, &UBattleActionMenu::OnSkillMenuClicked);
	}
	
	if (SkillListWidget)
	{
		SkillListWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
	
	if (ItemListWidget)
	{
		ItemListWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
	
	CurrentSubPanel = EActionMenuSubPanel::Main;
	bMainButtonsLocked = false;
	ApplySubPanelState();
}

void UBattleActionMenu::ShowMenu(ABattleAllyUnit* TargetUnit)
{
	CurrentUnit = TargetUnit;

	if (SkillListWidget)
	{
		SkillListWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
	
	if (ItemListWidget)
	{
		ItemListWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
	
	// 턴 시작마다 UI 상태 리셋
	CurrentSubPanel = EActionMenuSubPanel::Main;
	bMainButtonsLocked = false;
	ApplySubPanelState();
	
	// 서브 메뉴들은 일단 숨김
	
	
	// 턴 시작 때마다 스킬 목록 갱신
	RebuildSkillList();
	
	UpdateMenuPosition();
	
	// 카메라가 이동하는 동안 매 프레임 위치 갱신
	bIsFollowingUnit = true;
	
	SetVisibility(ESlateVisibility::Visible);
}

void UBattleActionMenu::SetMainButtonsEnabled(bool bEnabled)
{
	// 메인 버튼 3개 동시 잠금/해제
	if (Btn_Attack) Btn_Attack->SetIsEnabled(bEnabled);
	if (Btn_Skill) Btn_Skill->SetIsEnabled(bEnabled);
	if (Btn_Item) Btn_Item->SetIsEnabled(bEnabled);
}

void UBattleActionMenu::ApplySubPanelState()
{
	// 패널 표시 + 메인 버튼 활성 조건을 단일 함수에서 관리
	if (SkillListWidget)
	{
		SkillListWidget->SetVisibility(CurrentSubPanel == EActionMenuSubPanel::SkillList
			? ESlateVisibility::Visible
			: ESlateVisibility::Collapsed);
	}
	
	if (ItemListWidget)
	{
		ItemListWidget->SetVisibility(CurrentSubPanel == EActionMenuSubPanel::ItemList
			? ESlateVisibility::Visible
			: ESlateVisibility::Collapsed);
	}
	
	// 메인 버튼은 서브 패널이 Main이고 잠금이 없을 때만 활성화
	const bool bEnableMain = (CurrentSubPanel == EActionMenuSubPanel::Main) && !bMainButtonsLocked;
	SetMainButtonsEnabled(bEnableMain);
}

bool UBattleActionMenu::IsAnySubMenuOpen() const
{
	// PC에서 ESC 처리 분기용
	return CurrentSubPanel == EActionMenuSubPanel::SkillList
		|| CurrentSubPanel == EActionMenuSubPanel::ItemList;
}

bool UBattleActionMenu::TryCancelSubMenu()
{
	// ESC로 서브메뉴 취소 - 닫고 메인 버튼 복구
	if (!IsAnySubMenuOpen())
	{
		return false;
	}
	
	CurrentSubPanel = EActionMenuSubPanel::Main;
	bMainButtonsLocked = false; // 취소이므로 잠금 해제
	ApplySubPanelState();
	return true;
}

void UBattleActionMenu::UpdateMenuPosition()
{
	if (!CurrentUnit || !CurrentUnit->UIAnchorPoint) return;
	
	APlayerController* PC = GetOwningPlayer();
	if (!PC) return;
	
	FVector2D ScreenPosition;
	const FVector WorldLocation = CurrentUnit->UIAnchorPoint->GetComponentLocation();
	
	const bool bProjected = 
		UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(PC, WorldLocation, ScreenPosition, true);
	
	if (!bProjected)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ActionMenu] Project FAILED unit=%s world=%s"),
			*GetNameSafe(CurrentUnit), *WorldLocation.ToString());
		return;
	}
	
	// 화면 밖으로 나가면 클램프
	int32 VX = 0, VY = 0;
	PC->GetViewportSize(VX, VY);
	
	const float Margin = 20.f;
	ScreenPosition.X = FMath::Clamp(ScreenPosition.X, Margin, (float)VX - Margin);
	ScreenPosition.Y = FMath::Clamp(ScreenPosition.Y, Margin, (float)VY - Margin);
	
	SetPositionInViewport(ScreenPosition, false);
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
	
	// 공격 선택 순간부터 입력 잠금(중복 클릭 방지)
	bMainButtonsLocked = true; // // 공격 확정 ~ 타겟 선택/실행 동안 잠금
	CurrentSubPanel = EActionMenuSubPanel::Main;
	ApplySubPanelState();
	
	OnActionRequested.Broadcast(BasicAttackData);
}

void UBattleActionMenu::OnSkillMenuClicked()
{
	if (!SkillListWidget) return;
	
	// 스킬 메뉴는 "열기"만 지원(열린 상태에서 버튼은 disabled라 다시 클릭으로 닫기 불가)
	bMainButtonsLocked = true;                 // // 스킬 메뉴 열린 동안 메인 버튼 잠금
	CurrentSubPanel = EActionMenuSubPanel::SkillList;
	ApplySubPanelState();
	
	RebuildSkillList();
}

void UBattleActionMenu::HandleSkillSlotClicked(USkillDataAsset* Skill)
{
	if (!IsValid(Skill)) return;
	
	// 선택 후 리스트 닫기
	CurrentSubPanel = EActionMenuSubPanel::Main;

	bMainButtonsLocked = true; // 스킬 확정 이후 전투 상태 전환까지 잠금 유지
	ApplySubPanelState();
	
	OnSkillRequested.Broadcast(Skill);
}



void UBattleActionMenu::OnItemMenuClicked()
{
	if (!ItemListWidget) return;

	bMainButtonsLocked = true;
	CurrentSubPanel = EActionMenuSubPanel::ItemList;
	ApplySubPanelState();
	
	OnItemMenuRequested.Broadcast();
}

void UBattleActionMenu::RebuildSkillList()
{
	if (!SkillListWidget) return;
	SkillListWidget->ClearChildren();
	SpawnedSkillSlots.Empty();
	
	if (!CurrentUnit || !CurrentUnit->UnitData) return;
	if (!SkillSlotClass) return;
	
	const TArray<USkillDataAsset*>& Skills = CurrentUnit->UnitData->SkillList;
	const int32 StartIndex = 1; // 기본 공격이 0번에 default로 들어가 있기 때문에 1부터 시작
	
	for (int32 i = StartIndex; i < Skills.Num(); ++i)
	{
		USkillDataAsset* Skill = Skills[i];
		if (!IsValid(Skill)) continue;
		
		UBattleSkillSlotWidget* SkillSlot = CreateWidget<UBattleSkillSlotWidget>(GetOwningPlayer(), SkillSlotClass);
		if (!SkillSlot) continue;
		
		SkillSlot->Init(Skill, i);
		SkillSlot->OnSkillClicked.RemoveDynamic(this, &UBattleActionMenu::HandleSkillSlotClicked);
		SkillSlot->OnSkillClicked.AddDynamic(this, &UBattleActionMenu::HandleSkillSlotClicked);
		
		SkillListWidget->AddChild(SkillSlot);
		SpawnedSkillSlots.Add(SkillSlot);
	}
}


