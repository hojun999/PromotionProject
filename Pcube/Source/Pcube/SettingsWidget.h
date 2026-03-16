// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BaseHUD.h"
#include "SettingsWidget.generated.h"

class UProgressBar;
class UButton;
class UAudioManagerSubsystem;
class ABaseHUD;

/**
 * ESC 설정창 위젯
 * 
 * WBP에서 필요한 위젯 이름:
 *   PB_BGMVolume  (ProgressBar) - 배경음악 볼륨 표시
 *   PB_SFXVolume  (ProgressBar) - 효과음 볼륨 표시
 *   Btn_BGMUp     (Button)      - BGM +0.1
 *   Btn_BGMDown   (Button)      - BGM -0.1
 *   Btn_SFXUp     (Button)      - SFX +0.1
 *   Btn_SFXDown   (Button)      - SFX -0.1
 *   Btn_Close     (Button)      - 창 닫기
 */
UCLASS()
class PCUBE_API USettingsWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

public:
	void Open();
	void Close();
	bool IsOpen() const;

	// 소유 HUD 설정 - Btn_Play 동작 분기에 사용
	void SetOwningHUD(ABaseHUD* InHUD) { OwningHUD = InHUD; }
	
protected:
	UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
	TObjectPtr<UProgressBar> PB_BGMVolume = nullptr;

	UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
	TObjectPtr<UProgressBar> PB_SFXVolume = nullptr;

	// BindWidgetOptional: WBP에 없어도 컴파일 에러 없음
	UPROPERTY(BlueprintReadWrite, meta=(BindWidgetOptional))
	TObjectPtr<UButton> Btn_BGMUp = nullptr;

	UPROPERTY(BlueprintReadWrite, meta=(BindWidgetOptional))
	TObjectPtr<UButton> Btn_BGMDown = nullptr;

	UPROPERTY(BlueprintReadWrite, meta=(BindWidgetOptional))
	TObjectPtr<UButton> Btn_SFXUp = nullptr;

	UPROPERTY(BlueprintReadWrite, meta=(BindWidgetOptional))
	TObjectPtr<UButton> Btn_SFXDown = nullptr;

	UPROPERTY(BlueprintReadWrite, meta=(BindWidgetOptional))
	TObjectPtr<UButton> Btn_Close = nullptr;

	// L_MainMenu: 게임 시작 / L_OpenWorld, L_Battle : Resume // 추가됨
	UPROPERTY(BlueprintReadWrite, meta=(BindWidgetOptional))
	TObjectPtr<UButton> Btn_Play = nullptr; // Play/Resume

private:
	UFUNCTION() void OnBGMUp();
	UFUNCTION() void OnBGMDown();
	UFUNCTION() void OnSFXUp();
	UFUNCTION() void OnSFXDown();
	UFUNCTION() void OnCloseClicked();
	UFUNCTION() void OnPlayClicked(); // HUD 타입 판별 분기
	
	UAudioManagerSubsystem* GetAudioManager() const;
	void RefreshVolumeDisplay();
	void ResumeGame(); // OpenWorld/Battle 공통 resume 처리
	
	// 소유 HUD 참조
	UPROPERTY()
	TObjectPtr<ABaseHUD> OwningHUD = nullptr;
	
	static constexpr float VolumeStep = 0.1f;
};
