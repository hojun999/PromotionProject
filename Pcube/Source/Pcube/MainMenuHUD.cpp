// Fill out your copyright notice in the Description page of Project Settings.

#include "MainMenuHUD.h"
#include "AudioManagerSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

void AMainMenuHUD::BeginPlay()
{
	Super::BeginPlay();

	// 메인메뉴 BGM 재생
	if (MainMenuBGMSound)
	{
		if (UGameInstance* GI = GetGameInstance())
		{
			if (UAudioManagerSubsystem* AM = GI->GetSubsystem<UAudioManagerSubsystem>())
			{
				AM->PlayBGM(MainMenuBGMSound);
			}
		}
	}

	// 메인메뉴 진입 시 Settings 자동 오픈 (마우스 커서 활성)
	if (APlayerController* PC = GetOwningPlayerController())
	{
		PC->FlushPressedKeys();
		FInputModeGameAndUI Mode;
		Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		Mode.SetHideCursorDuringCapture(false);
		PC->SetInputMode(Mode);
		PC->bShowMouseCursor = true;
	}

	// Settings 위젯 열기 (BGM 설정 + 게임 시작 버튼)
	GetWorldTimerManager().SetTimer(
		TimerHandle_OpenSettings,
		[this]()
		{
			ToggleSettings();
		},
		0.2f, false
	);
}

void AMainMenuHUD::OnSettingsOpened()
{
	// 메인메뉴에서는 Pause 불필요 - 이미 정지 상태 없음
}

void AMainMenuHUD::OnSettingsClosed()
{
	// 메인메뉴에서 Close는 아무 동작 없음 (Btn_Play로만 진행)
	// 닫혔을 때 다시 열기 - 메인메뉴는 항상 Settings가 떠있어야 함
	GetWorldTimerManager().SetTimerForNextTick([this]()
	{
		if (!IsSettingsOpen())
		{
			ToggleSettings();
		}
	});
}

void AMainMenuHUD::OnGameStartRequested()
{
	// TODO: 튜토리얼 이미지 시퀀스 시작
	// 현재는 바로 OpenWorld로 전환
	StartOpenWorld();
}

void AMainMenuHUD::StartOpenWorld()
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UAudioManagerSubsystem* AM = GI->GetSubsystem<UAudioManagerSubsystem>())
		{
			AM->StopBGM(1.0f);
		}
	}

	// L_OpenWorld로 전환
	UGameplayStatics::OpenLevel(this, FName("L_OpenWorld"));
}