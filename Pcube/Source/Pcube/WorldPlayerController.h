// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "WorldPlayerController.generated.h"


UCLASS()
class PCUBE_API AWorldPlayerController : public APlayerController
{
	GENERATED_BODY()
	AWorldPlayerController();
	
public:
	
protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void OnPossess(APawn* InPawn) override;
	
private:
	void Input_ToggleInventory(); // I
	void Input_TogglePlayerInfo(); // Tab
	void Input_CloseUI(); // ESC
	
	UPROPERTY()
	class AWorldCCTVCameraDirector* CachedDirector = nullptr;

	FTimerHandle TimerHandle_TrySetCCTV;

	void TrySetCCTVView();
};
