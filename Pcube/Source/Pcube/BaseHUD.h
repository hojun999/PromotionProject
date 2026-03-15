// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "BaseHUD.generated.h"

class USettingsWidget;

UCLASS()
class PCUBE_API ABaseHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

	// 모든 레벨 HUD 공통 - ESC 설정창 토글
	UFUNCTION(BlueprintCallable, Category="UI")
	void ToggleSettings();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="UI")
	bool IsSettingsOpen() const;

	USettingsWidget* GetSettingsWidget() const { return SettingsWidget; } // 추가됨

protected:
	// BP_WorldHUD, BP_BattleHUD 에서 WBP_SettingsWidget 할당
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UI|Settings")
	TSubclassOf<USettingsWidget> SettingsWidgetClass = nullptr;

	UPROPERTY()
	TObjectPtr<USettingsWidget> SettingsWidget = nullptr;
};
