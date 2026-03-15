// Fill out your copyright notice in the Description page of Project Settings.

#include "SettingsWidget.h"
#include "AudioManagerSubsystem.h"
#include "Components/ProgressBar.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"

void USettingsWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// AddDynamic은 람다 래퍼 불가 - 직접 바인딩
	if (Btn_Close)   Btn_Close->OnClicked.AddDynamic(this,   &USettingsWidget::OnCloseClicked);
	if (Btn_BGMUp)   Btn_BGMUp->OnClicked.AddDynamic(this,   &USettingsWidget::OnBGMUp);
	if (Btn_BGMDown) Btn_BGMDown->OnClicked.AddDynamic(this, &USettingsWidget::OnBGMDown);
	if (Btn_SFXUp)   Btn_SFXUp->OnClicked.AddDynamic(this,   &USettingsWidget::OnSFXUp);
	if (Btn_SFXDown) Btn_SFXDown->OnClicked.AddDynamic(this, &USettingsWidget::OnSFXDown);
	if (Btn_Play)    Btn_Play->OnClicked.AddDynamic(this,    &USettingsWidget::OnCloseClicked); // Play = Close와 동일 동작

	// 볼륨 초기값 0.5 설정
	if (UAudioManagerSubsystem* AM = GetAudioManager())
	{
		if (AM->GetBGMVolume() == 1.0f) AM->SetBGMVolume(0.5f); // 기본값 그대로면 0.5로 설정
		if (AM->GetSFXVolume() == 1.0f) AM->SetSFXVolume(0.5f);
	}

	// 생성 직후 숨김
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
	// 닫힐 때 커서 끄고 게임 입력 복구
	if (APlayerController* PC = GetOwningPlayer())
	{
		FInputModeGameOnly Mode;
		PC->SetInputMode(Mode);
		PC->bShowMouseCursor = false;
	}
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

