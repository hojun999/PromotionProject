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
	
public:
	UBattleActionMenu* GetActionMenuWidget() const { return BattleActionMenuWidget; }
	
	void ShowActionMenu(ABattleAllyUnit* AllyUnit);
	void HideActionMenu();
	void ShowGameOverUI();
	void ShowVictoryUI();
	
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
	
	UPROPERTY()
	UBattleActionOrderWidget* ActionOrderWidget = nullptr;
	
	UPROPERTY()
	UBattleActionMenu* BattleActionMenuWidget = nullptr;
	
	UPROPERTY()
	UUserWidget* BattleStateNoticeWidget = nullptr;
	
	UPROPERTY()
	class UAllyStatusPanelWidget* AllyStatusPanelWidget = nullptr;
	
	UPROPERTY()
	TObjectPtr<UUserWidget> GameOverWidget = nullptr;
	
	UPROPERTY()
	TObjectPtr<UUserWidget> VictoryWidget = nullptr;
	
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
