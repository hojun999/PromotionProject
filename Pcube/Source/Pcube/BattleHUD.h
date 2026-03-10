// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseHUD.h"
#include "BattleControlSubsystem.h"
#include "BattleActionOrderWidget.h"
#include "BattleActionMenu.h"
#include "BattleActionIntentWidget.h"
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
	void ShowActionIntentForSkill(USkillDataAsset* Skill);
	void ShowActionIntentForItem(UItemDataAsset* Item);
	void HideActionIntent();
	
protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(EditAnywhere, Category="Battle|UI")
	TSubclassOf<UUserWidget> ActionOrderClass;
	
	UPROPERTY(EditAnywhere, Category="Battle|UI")
	TSubclassOf<UUserWidget> ActionMenuClass;
	
	UPROPERTY(EditAnywhere, Category="Battle|UI")
	TSubclassOf<UUserWidget> BattleStateNoticeClass;
	
	UPROPERTY(EditDefaultsOnly, Category="Battle|UI")
	TSubclassOf<UUserWidget> GameOverWidgetClass;
	
	UPROPERTY(EditDefaultsOnly, Category="Battle|UI")
	TSubclassOf<UUserWidget> VictoryWidgetClass;
	
	UPROPERTY(EditAnywhere, Category="Battle|UI")
	TSubclassOf<class UAllyStatusPanelWidget> AllyStatusPanelClass;
	
	// --- 생성된 위젯 인스턴스 참조 ---
	UPROPERTY()
	UBattleActionOrderWidget* ActionOrderWidget = nullptr;
	
	UPROPERTY()
	UBattleActionMenu* BattleActionMenuWidget = nullptr;
	
	UPROPERTY()
	UBattleActionIntentWidget* ActionIntentWidget = nullptr;
	
	UPROPERTY()
	UUserWidget* BattleStateNoticeWidget = nullptr;
	
	UPROPERTY()
	class UAllyStatusPanelWidget* AllyStatusPanelWidget = nullptr;
	
	UPROPERTY()
	TObjectPtr<UUserWidget> GameOverWidget = nullptr;
	
	UPROPERTY()
	TObjectPtr<UUserWidget> VictoryWidget = nullptr;
	
	UPROPERTY()
	TObjectPtr<UUserWidget> DisplayCancleWidget = nullptr;
	
	UPROPERTY()
	TObjectPtr<UUserWidget> DisplayChangeTargetWidget = nullptr;
	
	// --- 서브시스템 델리게이트 응답 ---
	UFUNCTION()
	void HandleBattleStateChanged(EBattleState NewState);

	UFUNCTION()
	void HandleBattleUnitSpawned(const TArray<AActor*>& Allies, const TArray<AActor*>& Enemies);
	
	UFUNCTION()
	void HandleTurnUnitChanged(ABattleBaseUnit* ActiveUnit);

	UFUNCTION()
	void HandleActionOrderChanged(const TArray<AActor*>& NewOrder);
	
	UFUNCTION()
	void HandleTargetChanged(AActor* NewTarget);

	UFUNCTION()
	void HandleBattleFinished(EBattleResult Result);
	
private:
	void CreateAllWidgets();
	void BindSubsystemEvents();
};
