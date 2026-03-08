// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "BattleGameMode.generated.h"

UCLASS()
class PCUBE_API ABattleGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	// 현재 전투 상태
	// UPROPERTY(BlueprintReadOnly, Category="Battle")
	// EBattleState CurrentState;
	ABattleGameMode();
	
	UPROPERTY(EditAnywhere, Category="Battle", meta=(DeprecatedProperty, DeprecationMessage="Use UBattleControlSubsystem instead."))
	TSubclassOf<AActor> BattleManager;
};