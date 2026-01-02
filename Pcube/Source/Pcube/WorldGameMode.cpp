// Fill out your copyright notice in the Description page of Project Settings.


#include "WorldGameMode.h"
#include "WorldAllyUnit.h"
#include "WorldPlayerController.h"

AWorldGameMode::AWorldGameMode()
{
	// 기본 폰 설정
	// 이후 BP 생성 후 에디터에서 교체 예정
	DefaultPawnClass = AWorldAllyUnit::StaticClass();
	
	// 플레이어 컨트롤러 등록
	PlayerControllerClass = AWorldPlayerController::StaticClass();
	
}
