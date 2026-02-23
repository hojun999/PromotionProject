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
	
	// 초기 상태: 3버튼 보임, 리스트 숨김
	CurrentView = EActionMenuView::Main;
	ApplyView();
}

void UBattleActionMenu::SetMainButtonsVisibility(ESlateVisibility NewVis)
{
	// 버튼3개를 Visibility로 숨기거나 보여주는 함수
	if (MainButtonsRoot)
	{
		MainButtonsRoot->SetVisibility(NewVis);
		return;
	}
	
	// 컨테이너가 없으면 개별 버튼 토글로 fallback
	if (Btn_Attack) Btn_Attack->SetVisibility(NewVis);
	if (Btn_Skill)  Btn_Skill->SetVisibility(NewVis);
	if (Btn_Item)   Btn_Item->SetVisibility(NewVis);
}

void UBattleActionMenu::ApplyView()
{
	// View 상태에 따라 3버튼과 리스트들의 Visibility를 일괄 갱신
	if (!SkillListWidget || !ItemListWidget) return;
	
	switch (CurrentView)
	{
	case EActionMenuView::Main:
		SetMainButtonsVisibility(ESlateVisibility::Visible);
		SkillListWidget->SetVisibility(ESlateVisibility::Collapsed);
		ItemListWidget->SetVisibility(ESlateVisibility::Collapsed);
		break;

	case EActionMenuView::SkillList:
		SetMainButtonsVisibility(ESlateVisibility::Collapsed);
		SkillListWidget->SetVisibility(ESlateVisibility::Visible);
		ItemListWidget->SetVisibility(ESlateVisibility::Collapsed);
		break;

	case EActionMenuView::ItemList:
		SetMainButtonsVisibility(ESlateVisibility::Collapsed);
		SkillListWidget->SetVisibility(ESlateVisibility::Collapsed);
		ItemListWidget->SetVisibility(ESlateVisibility::Visible);
		break;
	}
}


void UBattleActionMenu::ShowMenu(ABattleAllyUnit* TargetUnit)
{
	CurrentUnit = TargetUnit;
	
	// 턴 시작마다 메인 화면으로 리셋(=메인 버튼 다시 보이게)
	CurrentView = EActionMenuView::Main; // // 새 턴에는 항상 메인으로
	ApplyView();
	
	// 서브 메뉴들은 일단 숨김
	
	
	// 턴 시작 때마다 스킬 목록 갱신
	RebuildSkillList();
	
	UpdateMenuPosition();
	
	// 카메라가 이동하는 동안 매 프레임 위치 갱신
	bIsFollowingUnit = true;
	
	SetVisibility(ESlateVisibility::Visible);
}


bool UBattleActionMenu::IsSubMenuOpen() const
{
	// 스킬/아이템 리스트가 열려있는지 여부(ESC 취소 분기용)
	return CurrentView == EActionMenuView::SkillList || CurrentView == EActionMenuView::ItemList;
}

bool UBattleActionMenu::TryCancelSubMenu()
{
	// ESC로 서브메뉴 취소 - 닫고 메인 버튼 복구
	if (!IsSubMenuOpen())
	{
		return false;
	}
	
	CurrentView = EActionMenuView::Main;
	ApplyView();
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
	// 공격은 PC에서 HUD->HideActionMenu() 호출 중이므로 여기서는 브로드캐스트만
	OnActionRequested.Broadcast(BasicAttackData);
}

void UBattleActionMenu::OnSkillMenuClicked()
{
	// 스킬 버튼 클릭 -> 메인 3버튼 숨기고 스킬 리스트 표시
	CurrentView = EActionMenuView::SkillList; // // 스킬 리스트 화면 진입
	ApplyView();

	RebuildSkillList();
}

void UBattleActionMenu::HandleSkillSlotClicked(USkillDataAsset* Skill)
{
	if (!IsValid(Skill)) return;
	
	// 스킬 선택은 확정이므로, 리스트만 닫고 메인 버튼을 다시 보여주지 않음
	// -> 공격 버튼과 동일하게 메뉴 자체를 숨겨서 중복 입력을 근본 차단
	CurrentView = EActionMenuView::Main; // 다음번 ShowMenu에서 정상 복구될 수 있게 내부 상태는 메인으로
	ApplyView();

	bIsFollowingUnit = false;                 // 타겟 선택/연출 동안 UI 위치 추적 중단
	SetVisibility(ESlateVisibility::Collapsed); // 스킬 확정 후 액션 메뉴 숨김(중복 클릭 방지)

	OnSkillRequested.Broadcast(Skill);
}



void UBattleActionMenu::OnItemMenuClicked()
{
	// 아이템 버튼 클릭 -> 메인 3버튼 숨기고 아이템 리스트 표시
	CurrentView = EActionMenuView::ItemList; // // 아이템 리스트 화면 진입
	ApplyView();

	OnItemMenuRequested.Broadcast(); // 아이템 리스트 빌드는 추후(인벤 구현 후) 연결
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


