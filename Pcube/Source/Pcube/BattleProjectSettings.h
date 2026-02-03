// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "BattleProjectSettings.generated.h"

UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="Battle System Settings"))
class PCUBE_API UBattleProjectSettings : public UDeveloperSettings
{
	GENERATED_BODY()
	
public:
	//UBattleProjectSettings();
	
	UPROPERTY(Config, EditAnywhere, Category="Initial Data")
	TSoftObjectPtr<class UAllyPartyDataAsset> DefaultAllyPartyAsset;
};
