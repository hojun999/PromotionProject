// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BattleActionMenu.generated.h"

class ABattleAllyUnit;
class UButton;

UCLASS()
class PCUBE_API UBattleActionMenu : public UUserWidget
{
	GENERATED_BODY()
	
public:
	// HUD에서 호출 - 메뉴 초기화 및 표시
	void ShouMenu(ABattleAllyUnit* TargetUnit);
	
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
	UUserWidget* SkillListWidget;
	// class USkillListWidget* SkillListWidget;
	
	UPROPERTY(meta = (BindWidget))
	UUserWidget* ItemListWidget;
	// class UItemListWidget* ItemListWidget;
	
	// 공격은 바로 적 지정 및 실행
	
	// --- 이벤트 핸들러 ---
	UFUNCTION()
	void OnAttackClicked();
	
	UFUNCTION()
	void OnSkillMenuClicked();
	
	UFUNCTION()
	void OnItemMunuClicked();
	
private:
	UPROPERTY()
	ABattleAllyUnit* CurrentUnit;
};
