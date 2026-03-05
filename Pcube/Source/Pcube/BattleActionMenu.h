// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BattleAllyUnit.h"
#include "BattleSkillSlotWidget.h"
#include "BattleActionMenu.generated.h"

class ABattleAllyUnit;
class UButton;
class UVerticalBox;
class UWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActionRequested, USkillDataAsset*, SkillDataAsset);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSkillMenuRequested);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSkillRequested, USkillDataAsset*, Skill);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnItemMenuRequested);

UENUM(BlueprintType)
enum class EActionMenuView : uint8
{
	Main UMETA(DisplayName="Main"),				// 3버튼 보임, 리스트 숨김
	SkillList UMETA(DisplayName="SkillList"),	// 스킬 리스트 보임, 3버튼 숨김
	ItemList UMETA(DisplayName="ItemList"),		// 아이템 리스트 보임, 3버튼 숨김
};


UCLASS()
class PCUBE_API UBattleActionMenu : public UUserWidget
{
	GENERATED_BODY()
	
public:
	// HUD에서 호출 - 메뉴 초기화 및 표시
	UFUNCTION(BlueprintCallable)
	void ShowMenu(ABattleAllyUnit* TargetUnit);
	
	void UpdateMenuPosition();
	void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	
	// ESC 등으로 열린 서브메뉴를 닫고 메인 버튼을 다시 활성화
	UFUNCTION(BlueprintCallable)
	bool TryCancelSubMenu(); // 서브메뉴가 열려있으면 닫기 - 취소 입력 처리용
	
	UFUNCTION(BlueprintCallable)
	bool IsSubMenuOpen() const; // 서브메뉴 열림 여뷰 - PC에서 분기
	
	// --- 델리게이트 ---
	UPROPERTY(BlueprintAssignable)
	FOnActionRequested OnActionRequested;
	
	UPROPERTY(BlueprintAssignable)
	FOnSkillMenuRequested OnSkillMenuRequested;
	
	UPROPERTY(BlueprintAssignable, Category="Battle|UI")
	FOnSkillRequested OnSkillRequested;
	
	UPROPERTY(BlueprintAssignable)
	FOnItemMenuRequested OnItemMenuRequested;
	
protected:
	virtual void NativeConstruct() override;
	
	// --- 메인 버튼 ---
	UPROPERTY(meta = (BindWidget))
	UButton* Btn_Attack;
	
	UPROPERTY(meta = (BindWidget))
	UButton* Btn_Skill;
	
	UPROPERTY(meta = (BindWidget))
	UButton* Btn_Item;
	
	// 3버튼을 담는 컨테이너
	UPROPERTY(meta=(BindWidget))
	UWidget* MainButtonsRoot = nullptr;		// 3버튼을 한 번에 숨기기 위한 루트 위젯
	
	// --- 하위 패널 (하위 위젯들) --- 	
	UPROPERTY(meta = (BindWidget))
	UVerticalBox* SkillListWidget;
	// class USkillListWidget* SkillListWidget;
	
	UPROPERTY(meta = (BindWidget))
	UVerticalBox* ItemListWidget;
	// class UItemListWidget* ItemListWidget;
	
	// --- 이벤트 핸들러 ---
	UFUNCTION(BlueprintCallable)
	void OnAttackClicked();
	
	UFUNCTION(BlueprintCallable)
	void OnSkillMenuClicked();
	
	UFUNCTION(BlueprintCallable)
	void OnItemMenuClicked();
	
	UFUNCTION()
	void HandleSkillSlotClicked(USkillDataAsset* Skill);
	
	void RebuildSkillList();
	
	// --- 스킬 데이터 에셋 ---
	UPROPERTY(EditAnywhere, Category="Battle|Data")
	USkillDataAsset* BasicAttackData;
	
	UPROPERTY(EditAnywhere, Category="Battle|UI")
	TSubclassOf<UBattleSkillSlotWidget> SkillSlotClass;
private:
	// --- 함수 ---
	
	// 현재 뷰 상태를 Visibility에 반영
	void ApplyView(); // Main/SkillList/ItemList 상태에 따라 버튼/리스트 Visibility를 일괄 갱신
	
	// 메인 버튼 3개를 한 번에 Visible/Collapsed
	void SetMainButtonsVisibility(ESlateVisibility NewVis);
	
	
	// --- 변수 ---
	UPROPERTY()
	TObjectPtr<ABattleAllyUnit> CurrentUnit;
	
	UPROPERTY()
	TArray<TObjectPtr<UBattleSkillSlotWidget>> SpawnedSkillSlots;
	
	UPROPERTY()
	EActionMenuView CurrentView = EActionMenuView::Main;	// 현재 메뉴 뷰 상태(메인/스킬리스트/아이템리스트)
	
	bool bIsFollowingUnit;
	
	// 임시 피드백 출력(현재: OnScreenDebugMessage) - 나중에 텍스트 위젯으로 교체하기 위한 훅
	void ShowTempFeedbackMessage(const FString& Message, float Duration = 1.5f) const; // SP 부족 등 피드백 표시용
};
