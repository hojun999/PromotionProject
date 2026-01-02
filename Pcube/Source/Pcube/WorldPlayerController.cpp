// Fill out your copyright notice in the Description page of Project Settings.


#include "WorldPlayerController.h"

void AWorldPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	// 이동 레벨에서는 마우스 커서 숨기기
	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);
	bShowMouseCursor = false;
}