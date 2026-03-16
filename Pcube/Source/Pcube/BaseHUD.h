// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "BaseHUD.generated.h"

class USettingsWidget;

UENUM(BlueprintType)
enum class EHUDType : uint8
{
	MainMenu   UMETA(DisplayName="MainMenu"),
	OpenWorld  UMETA(DisplayName="OpenWorld"),
	Battle     UMETA(DisplayName="Battle"),
};

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

	// HUD 타입 반환 - 서브클래스에서 오버라이드
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="UI")
	virtual EHUDType GetHUDType() const { return EHUDType::OpenWorld; }
	
	// Pause 처리
	virtual void OnSettingsOpened() {}   // 열릴 때 레벨별 처리
	virtual void OnSettingsClosed() {}   // 닫힐 때 레벨별 처리
	
protected:
	// BP_WorldHUD, BP_BattleHUD 에서 WBP_SettingsWidget 할당
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UI|Settings")
	TSubclassOf<USettingsWidget> SettingsWidgetClass = nullptr;

	UPROPERTY()
	TObjectPtr<USettingsWidget> SettingsWidget = nullptr;
};
