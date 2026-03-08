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
	
	if (Btn_Item)
	{
		Btn_Item->OnClicked.RemoveDynamic(this, &UBattleActionMenu::OnItemMenuClicked);
		Btn_Item->OnClicked.AddDynamic(this, &UBattleActionMenu::OnItemMenuClicked);
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
	SetMainButtonsVisibility(CurrentView == EActionMenuView::Main ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	
	if (SkillListWidget)
	{
		SkillListWidget->SetVisibility(CurrentView == EActionMenuView::SkillList
			? ESlateVisibility::Visible
			: ESlateVisibility::Collapsed);
	}
	
	if (ItemListWidget)
	{
		ItemListWidget->SetVisibility(CurrentView == EActionMenuView::ItemList
			? ESlateVisibility::Visible
			: ESlateVisibility::Collapsed);
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

	// 기본 공격은 "현재 유닛"의 SkillList[0]을 사용한다.
	// (구버전 호환) SkillList가 비어있으면 BasicAttackData(수동 지정)를 fallback.
	USkillDataAsset* BasicAtk = GetBasicAttackForCurrentUnit();
	if (!IsValid(BasicAtk))
	{
		ShowTempFeedbackMessage(TEXT("기본 공격 스킬이 지정되지 않았습니다."), 1.7f);
		UE_LOG(LogTemp, Error, TEXT("[ActionMenu] Basic attack skill is null. unit=%s"), *GetNameSafe(CurrentUnit));
		return;
	}

	// (선택) 기본 공격도 SP 비용을 사용할 수 있으므로 동일하게 검사
	if (!IsValid(CurrentUnit) || !CurrentUnit->CanUseSkill(BasicAtk))
	{
		const int32 CurSP = IsValid(CurrentUnit) ? CurrentUnit->GetCurrentSkillPoints() : 0;
		const int32 MaxSP = IsValid(CurrentUnit) ? CurrentUnit->GetMaxSkillPoints() : 0;
		const int32 Cost  = BasicAtk->SkillPointCost;

		ShowTempFeedbackMessage(
			FString::Printf(TEXT("스킬 포인트가 부족합니다: %s (필요 %d / 현재 %d/%d)"), *BasicAtk->SkillName, Cost, CurSP, MaxSP),
			1.7f);
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[ActionMenu] BasicAtk=%s path=%s"),
		*GetNameSafe(BasicAtk), BasicAtk ? *BasicAtk->GetPathName() : TEXT("None"));

	// 공격은 PC에서 HUD->HideActionMenu() 호출 중이므로 여기서는 브로드캐스트만
	OnActionRequested.Broadcast(BasicAtk);
}

USkillDataAsset* UBattleActionMenu::GetBasicAttackForCurrentUnit() const
{
	if (IsValid(CurrentUnit) && IsValid(CurrentUnit->UnitData))
	{
		const TArray<USkillDataAsset*>& Skills = CurrentUnit->UnitData->SkillList;
		if (Skills.Num() > 0 && IsValid(Skills[0]))
		{
			return Skills[0];
		}
	}

	// 구버전 호환: 메뉴 자체에 수동 지정된 BasicAttackData가 있으면 fallback
	return BasicAttackData;
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
	
	// SP 부족 여부를 먼저 검사, 부족하면 스킬 리스트 유지
	// SP가 부족하면 메시지 출력 + 스킬리스트 유지 (아무것도 닫지 않고 return)
	if (!IsValid(CurrentUnit) || !CurrentUnit->CanUseSkill(Skill))
	{
		const int32 CurSP = IsValid(CurrentUnit) ? CurrentUnit->GetCurrentSkillPoints() : 0;
		const int32 MaxSP = IsValid(CurrentUnit) ? CurrentUnit->GetMaxSkillPoints() : 0;
		const int32 Cost  = Skill->SkillPointCost;

		const FString Msg = FString::Printf(
			TEXT("스킬 포인트가 부족합니다: %s (필요 %d / 현재 %d/%d)"),
			*Skill->SkillName, Cost, CurSP, MaxSP
		);

		ShowTempFeedbackMessage(Msg, 1.7f);
		return; // ✅ 리스트/뷰 상태 유지
	}
	
	// 스킬 선택은 확정이므로, 리스트만 닫고 메인 버튼을 다시 보여주지 않음
	// -> 공격 버튼과 동일하게 메뉴 자체를 숨겨서 중복 입력을 근본 차단
	CurrentView = EActionMenuView::Main; // 다음번 ShowMenu에서 정상 복구될 수 있게 내부 상태는 메인으로
	ApplyView();

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

void UBattleActionMenu::ShowTempFeedbackMessage(const FString& Message, float Duration) const
{
#if !UE_BUILD_SHIPPING
	if (GEngine)
	{
		// 같은 키로 계속 갱신되게 해서 스팸 방지
		const int32 MsgKey = 77701; // "UI Feedback" 전용 키
		GEngine->AddOnScreenDebugMessage(MsgKey, Duration, FColor::Yellow, Message);
	}
#endif
}
