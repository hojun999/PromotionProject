// Fill out your copyright notice in the Description page of Project Settings.


#include "MyPlayerController.h"
#include "MyGameModeBase.h"

AMyGameModeBase::AMyGameModeBase()
{
	// 이 게임모드에서 사용할 컨트롤러 클래스를 지정
	PlayerControllerClass = AMyPlayerController::StaticClass();
}