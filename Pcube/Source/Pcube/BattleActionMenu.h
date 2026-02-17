// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BattleAllyUnit.h"
#include "BattleActionMenu.generated.h"

class ABattleAllyUnit;
class UButton;
class UVerticalBox;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActionRequested, USkillDataAsset*, SkillDataAsset);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSkillMenuRequested);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnItemMenuRequested);

UCLASS()
class PCUBE_API UBattleActionMenu : public UUserWidget
{
	GENERATED_BODY()
	
public:
	// HUD에서 호출 - 메뉴 초기화 및 표시
	void ShowMenu(ABattleAllyUnit* TargetUnit);
	void UpdateMenuPosition();
	void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	
	UPROPERTY(BlueprintAssignable)
	FOnActionRequested OnActionRequested;
	
	UPROPERTY(BlueprintAssignable)
	FOnSkillMenuRequested OnSkillMenuRequested;
	
	UPROPERTY(BlueprintAssignable)
	FOnItemMenuRequested OnItemMenuRequested;
	
	// --- 이벤트 핸들러 ---
	UFUNCTION(BlueprintCallable)
	void OnAttackClicked();
	
	UFUNCTION(BlueprintCallable)
	void OnSkillMenuClicked();
	
	UFUNCTION(BlueprintCallable)
	void OnItemMenuClicked();
	
protected:
	// --- 메인 버튼 ---
	UPROPERTY(meta = (BindWidget))
	UButton* Btn_Attack;
	
	UPROPERTY(meta = (BindWidget))
	UButton* Btn_Skill;
	
	UPROPERTY(meta = (BindWidget))
	UButton* Btn_Item;
	
	// --- 하위 패널 (하위 위젯들) --- 	
	UPROPERTY(meta = (BindWidget))
	UVerticalBox* SkillListWidget;
	// class USkillListWidget* SkillListWidget;
	
	UPROPERTY(meta = (BindWidget))
	UVerticalBox* ItemListWidget;
	// class UItemListWidget* ItemListWidget;
	
	
	// --- 스킬 데이터 에셋 ---
	UPROPERTY(EditAnywhere, Category="Battle|Data")
	USkillDataAsset* BasicAttackData;
	
private:
	UPROPERTY()
	ABattleAllyUnit* CurrentUnit;
	
	bool bIsFollowingUnit;
};
