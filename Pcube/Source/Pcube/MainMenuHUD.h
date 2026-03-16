// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseHUD.h"
#include "Sound/SoundBase.h"
#include "MainMenuHUD.generated.h"

/**
 * L_MainMenu 전용 HUD
 * - 게임 시작 버튼 → 튜토리얼 → L_OpenWorld 전환
 * - 오디오 설정창 (SettingsWidget 공유)
 */
UCLASS()
class PCUBE_API AMainMenuHUD : public ABaseHUD
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	virtual EHUDType GetHUDType() const override { return EHUDType::MainMenu; }

	// SettingsWidget의 Btn_Play에서 호출됨
	void OnGameStartRequested();

	// 추후 튜토리얼 완료 후 호출
	UFUNCTION(BlueprintCallable, Category="MainMenu")
	void StartOpenWorld();

protected:
	virtual void OnSettingsOpened() override;
	virtual void OnSettingsClosed() override;

	// 메인메뉴 BGM
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Audio")
	TObjectPtr<USoundBase> MainMenuBGMSound = nullptr;

	FTimerHandle TimerHandle_OpenSettings;
};
