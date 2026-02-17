// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseHUD.h"
#include "BattleControlSubsystem.h"
#include "BattleActionOrderWidget.h"
#include "BattleActionMenu.h"
#include "BattleHUD.generated.h"

class ABattleBaseUnit;
class UUserWidget;

UCLASS()
class PCUBE_API ABattleHUD : public ABaseHUD
{
	GENERATED_BODY()
	
	// TODO: 1. 턴 순서 2. 스킬 패널 3. 전투 상태 (전투 시작, 전투 종료 등) 4. 유닛 초상화 & 해당 유닛의 스탯 정보
	
public:
	UBattleActionMenu* GetActionMenuWidget() const { return BattleActionMenuWidget; }
	
	void ShowActionMenu(ABattleAllyUnit* AllyUnit);
	void HideActionMenu();
	void ShowGameOverUI();
	void ShowVictoryUI();
	
protected:
	virtual void BeginPlay() override;
	
	// --- 에디터에서 할당할 위젯 클래스들 ---
	UPROPERTY(EditAnywhere, Category="Battle|UI")
	TSubclassOf<UUserWidget> ActionOrderClass;
	
	UPROPERTY(EditAnywhere, Category="Battle|UI")
	TSubclassOf<UUserWidget> ActionMenuClass;
	
	UPROPERTY(EditAnywhere, Category="Battle|UI")
	TSubclassOf<UUserWidget> BattleStateNoticeClass;
	
	UPROPERTY(EditAnywhere, Category="Battle|UI")
	TSubclassOf<UUserWidget> AllyUnitInfoClass;
	
	UPROPERTY(EditDefaultsOnly, Category="Battle|UI")
	TSubclassOf<UUserWidget> GameOverWidgetClass;
	
	UPROPERTY(EditDefaultsOnly, Category="Battle|UI")
	TSubclassOf<UUserWidget> VictoryWidgetClass;
	
	// --- 생성된 위젯 인스턴스 참조 ---
	// 그냥 클래스 참조랑 TObjectPtr이랑 뭐가 다른거?
	UPROPERTY()
	UBattleActionOrderWidget* ActionOrderWidget;
	
	UPROPERTY()
	UBattleActionMenu* BattleActionMenuWidget;
	
	UPROPERTY()
	UUserWidget* BattleStateNoticeWidget;
	
	UPROPERTY()
	UUserWidget* AllyUnitInfoWidget;
	
	UPROPERTY()
	TObjectPtr<UUserWidget> GameOverWidget;
	
	UPROPERTY()
	TObjectPtr<UUserWidget> VictoryWidget;
	
	// --- 서브시스템 델리게이트 응답 ---
	UFUNCTION()
	void HandleBattleStateChanged(EBattleState NewState);

	UFUNCTION()
	void HandleTurnUnitChanged(ABattleBaseUnit* ActiveUnit);

	UFUNCTION()
	void HandleActionOrderChanged(const TArray<AActor*>& NewOrder);
	
	UFUNCTION()
	void HandleAllyUnitInfoUpdate(); // 1. 아군 유닛의 초상화 Image 띄우기 2. 전투 진행 상황 반영하여 아군 유닛 스탯(hp, mp) 갱신
	
	UFUNCTION()
	void HandleTargetChanged(AActor* NewTarget);
	
private:
	void CreateAllWidgets();
	void BindSubsystemEvents();
};
