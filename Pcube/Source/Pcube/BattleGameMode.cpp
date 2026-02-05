// Fill out your copyright notice in the Description page of Project Settings.


#include "BattleGameMode.h"

#include "BattleHUD.h"

ABattleGameMode::ABattleGameMode()
{
	// BattleGameMode 활성화 시 자동으로 ABattleHUD 사용
	HUDClass = ABattleHUD::StaticClass();
}
