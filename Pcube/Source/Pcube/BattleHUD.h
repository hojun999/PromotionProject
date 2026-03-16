// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseHUD.h"
#include "BattleControlSubsystem.h"
#include "BattleActionOrderWidget.h"
#include "BattleActionMenu.h"
#include "BattleActionIntentWidget.h"
#include "Sound/SoundBase.h"
#include "BattleHUD.generated.h"

class ABattleBaseUnit;
class UUserWidget;

UCLASS()
class PCUBE_API ABattleHUD : public ABaseHUD
{
	GENERATED_BODY()
	
	// TODO: 1. 턴 순서 2. 스킬 패널 3. 전투 상태 (전투 시작, 전투 종료 등) 4. 유닛 초상화 & 해당 유닛의 스탯 정보
	
public:
	// 전투 BGM (에디터 BP_BattleHUD 에서 할당)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Audio")
	TObjectPtr<USoundBase> BattleBGMSound = nullptr;

	UBattleActionMenu* GetActionMenuWidget() const { return BattleActionMenuWidget; }
	virtual EHUDType GetHUDType() const override { return EHUDType::Battle; }
	
	void ShowActionMenu(ABattleAllyUnit* AllyUnit);
	void HideActionMenu();
	void ShowGameOverUI();
	void ShowVictoryUI();
	void ShowGameClearUI();
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
	
	// 게임 클리어 위젯 (보스 처치 시)
	UPROPERTY(EditDefaultsOnly, Category="Battle|UI")
	TSubclassOf<UUserWidget> GameClearWidgetClass;
	
	// Q/E 타겟 변경 힌트 위젯 클래스
	UPROPERTY(EditDefaultsOnly, Category="Battle|UI")
	TSubclassOf<UUserWidget> ChangeTargetHintWidgetClass;

	// 우클릭 돌아가기 힌트 위젯 클래스
	UPROPERTY(EditDefaultsOnly, Category="Battle|UI")
	TSubclassOf<UUserWidget> CancelHintWidgetClass;
	
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
	TObjectPtr<UUserWidget> GameClearWidget = nullptr;
	
	UPROPERTY()
	TObjectPtr<UUserWidget> DisplayCancelWidget = nullptr;
	
	UPROPERTY()
	TObjectPtr<UUserWidget> DisplayChangeTargetWidget = nullptr;
	
	// --- 서브시스템 델리게이트 응답 ---
	UFUNCTION()
	void HandleBattleStateChanged(EBattleState NewState);

	UFUNCTION()
	void HandleBattleUnitSpawned(const TArray<AActor*>& Allies, const TArray<AActor*>& Enemies);
	
	UFUNCTION()
	void HandleTurnUnitChanged(ABattleBaseUnit* ActiveUnit);
	
	// 현재 턴 유닛 캐싱 - 아군/적군 판별용
	UPROPERTY()
	TObjectPtr<ABattleBaseUnit> CurrentTurnUnit = nullptr;
	
	UFUNCTION()
	void HandleActionOrderChanged(const TArray<AActor*>& NewOrder);
	
	UFUNCTION()
	void HandleTargetChanged(AActor* NewTarget);

	// 살아있는 적이 2개 이상인지 확인 (QE 힌트 표시 조건)
	bool HasMultipleAliveEnemies() const;
	
	UFUNCTION()
	void HandleBattleFinished(EBattleResult Result);
	
protected:
	virtual void OnSettingsOpened() override; // 추가됨
	virtual void OnSettingsClosed() override; // 추가됨
	
private:
	void CreateAllWidgets();
	void BindSubsystemEvents();
};
