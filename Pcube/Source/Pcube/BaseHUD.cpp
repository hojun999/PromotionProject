// Fill out your copyright notice in the Description page of Project Settings.

#include "BaseHUD.h"
#include "SettingsWidget.h"

void ABaseHUD::BeginPlay()
{
	Super::BeginPlay();

	if (!SettingsWidgetClass) return;

	SettingsWidget = CreateWidget<USettingsWidget>(GetWorld(), SettingsWidgetClass);
	if (SettingsWidget)
	{
		SettingsWidget->AddToViewport(100); // ZOrder 100: 다른 UI보다 위
		SettingsWidget->SetOwningHUD(this); // HUD 참조 전달
		SettingsWidget->Close();
	}
}

void ABaseHUD::ToggleSettings()
{
	if (!SettingsWidget) return;

	if (SettingsWidget->IsOpen())
	{
		SettingsWidget->Close();
		OnSettingsClosed(); // 닫힐 때 레벨별 처리
	}
	else
	{
		SettingsWidget->Open();
		OnSettingsOpened(); // 열릴 때 레벨별 처리
		// 열릴 때 커서 표시
		if (APlayerController* PC = GetOwningPlayerController())
		{
			FInputModeGameAndUI Mode;
			Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			PC->SetInputMode(Mode);
			PC->bShowMouseCursor = true;
		}
	}
}

bool ABaseHUD::IsSettingsOpen() const
{
	return SettingsWidget && SettingsWidget->IsOpen();
}
