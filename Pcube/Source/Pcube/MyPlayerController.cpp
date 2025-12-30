// Fill out your copyright notice in the Description page of Project Settings.


#include "MyPlayerController.h"

AMyPlayerController::AMyPlayerController()
{
	// 마우스 커서 모양 설정 - 이후 커스텀으로 변경
	DefaultMouseCursor = EMouseCursor::Crosshairs;
}

void AMyPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	// 입력 모드를 Game과 UI 동시 사용으로 설정
	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputMode.SetHideCursorDuringCapture(false);
	SetInputMode(InputMode);
	
	// 마우스 커서 활성화
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
	
	
}
