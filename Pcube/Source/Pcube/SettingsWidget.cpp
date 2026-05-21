// Fill out your copyright notice in the Description page of Project Settings.

#include "SettingsWidget.h"
#include "AudioManagerSubsystem.h"
#include "MainMenuHUD.h"
#include "Components/ProgressBar.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"

void USettingsWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (Btn_Close)   Btn_Close->OnClicked.AddDynamic(this,   &USettingsWidget::OnCloseClicked);
	if (Btn_Play)    Btn_Play->OnClicked.AddDynamic(this,    &USettingsWidget::OnPlayClicked);
	if (Btn_BGMUp)   Btn_BGMUp->OnClicked.AddDynamic(this,   &USettingsWidget::OnBGMUp);
	if (Btn_BGMDown) Btn_BGMDown->OnClicked.AddDynamic(this, &USettingsWidget::OnBGMDown);
	if (Btn_SFXUp)   Btn_SFXUp->OnClicked.AddDynamic(this,   &USettingsWidget::OnSFXUp);
	if (Btn_SFXDown) Btn_SFXDown->OnClicked.AddDynamic(this, &USettingsWidget::OnSFXDown);

	// 레벨 이동 후에도 Subsystem의 값을 유지하기 위해 강제 설정 로직 삭제
	RefreshVolumeDisplay(); // 현재 Subsystem 값으로 UI 동기화

	SetVisibility(ESlateVisibility::Collapsed);
}

void USettingsWidget::Open()
{
	RefreshVolumeDisplay();
	SetVisibility(ESlateVisibility::Visible);
}

void USettingsWidget::Close()
{
	SetVisibility(ESlateVisibility::Collapsed);
}

bool USettingsWidget::IsOpen() const
{
	return GetVisibility() == ESlateVisibility::Visible;
}

UAudioManagerSubsystem* USettingsWidget::GetAudioManager() const
{
	UGameInstance* GI = GetGameInstance();
	return GI ? GI->GetSubsystem<UAudioManagerSubsystem>() : nullptr;
}

void USettingsWidget::RefreshVolumeDisplay()
{
	UAudioManagerSubsystem* AM = GetAudioManager();
	if (!AM) return;

	if (PB_BGMVolume) PB_BGMVolume->SetPercent(AM->GetBGMVolume());
	if (PB_SFXVolume) PB_SFXVolume->SetPercent(AM->GetSFXVolume());
}

void USettingsWidget::OnBGMUp()
{
	if (UAudioManagerSubsystem* AM = GetAudioManager())
	{
		AM->SetBGMVolume(AM->GetBGMVolume() + VolumeStep);
		RefreshVolumeDisplay();
	}
}

void USettingsWidget::OnBGMDown()
{
	if (UAudioManagerSubsystem* AM = GetAudioManager())
	{
		AM->SetBGMVolume(AM->GetBGMVolume() - VolumeStep);
		RefreshVolumeDisplay();
	}
}

void USettingsWidget::OnSFXUp()
{
	if (UAudioManagerSubsystem* AM = GetAudioManager())
	{
		AM->SetSFXVolume(AM->GetSFXVolume() + VolumeStep);
		RefreshVolumeDisplay();
	}
}

void USettingsWidget::OnSFXDown()
{
	if (UAudioManagerSubsystem* AM = GetAudioManager())
	{
		AM->SetSFXVolume(AM->GetSFXVolume() - VolumeStep);
		RefreshVolumeDisplay();
	}
}

void USettingsWidget::OnCloseClicked()
{
	Close();
}

void USettingsWidget::OnPlayClicked()
{
	if (!OwningHUD) return;

	// HUD 타입에 따라 분기
	switch (OwningHUD->GetHUDType())
	{
	case EHUDType::MainMenu:
		// L_MainMenu: 게임 시작 - MainMenuHUD에 위임
		if (AMainMenuHUD* MainHUD = Cast<AMainMenuHUD>(OwningHUD))
		{
			MainHUD->OnGameStartRequested();
		}
		break;

	case EHUDType::OpenWorld:
	case EHUDType::Battle:
		// L_OpenWorld / L_Battle: Resume
		ResumeGame();
		break;
	}
}

void USettingsWidget::ResumeGame()
{
	Close();
	UGameplayStatics::SetGamePaused(this, false);
	if (OwningHUD)
	{
		OwningHUD->OnSettingsClosed();
	}
}