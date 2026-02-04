// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BattleControlSubsystem.h"
#include "BattleTurnOrderSlot.h"
#include "Blueprint/UserWidget.h"
#include "BattleHUDWidget.generated.h"

class UVerticalBox;
class UTextBlock;
class UCanvasPanel;
class UImage;
/**
 * 
 */
UCLASS()
class PCUBE_API UBattleHUDWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	// 위젯 초기화 시 서브시스템 바인딩
	virtual void NativeConstruct() override;
	
	// --- UI 컴포넌트 (블루프린트 위젯 이름과 일치해야 함) ---
	UPROPERTY(meta=(BindWidget))
	UVerticalBox* UnitActionOrderSlots; // 각 턴마다의 유닛 행동 순서 알림
	
	UPROPERTY(meta=(BindWidget))
	UTextBlock* StateNoticeText; // 현재 전투 상태 알림 (플레이어 턴 시작, 승리, 패배)
		
	UPROPERTY(meta=(BindWidget))
	UImage* UnitPortrait;	// 유닛 초상화 UI
	
	UPROPERTY(meta=(BindWidget))
	UCanvasPanel* SkillPanel; // 아군 턴일 때, 해당 아군의 스킬 창
	
	// --- 에디터에서 할당할 위젯 항목들 ---
	UPROPERTY(EditAnywhere, Category="Battle|UI")
	TSubclassOf<UBattleTurnOrderSlot> TurnSlotClass;
	
	// --- 함수 ---
	void RefreshTurnOrderList(const TArray<AActor*>& TurnOrder);
	
	// --- 델리게이트 응답 함수 ---
	UFUNCTION()
	void OnBattleStateChanged(EBattleState NewState);
	
	UFUNCTION()
	void OnTurnUnitChagned(ABattleBaseUnit* ActiveUnit);
	
private:
	// 상태에 따른 UI 가시성 업데이트
	void UpdateUIState(EBattleState State);
};
