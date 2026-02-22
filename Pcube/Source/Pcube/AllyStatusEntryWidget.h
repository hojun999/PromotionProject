// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BattleBaseUnit.h"
#include "AllyStatusEntryWidget.generated.h"

UCLASS()
class PCUBE_API UAllyStatusEntryWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	void InitWithUnit(ABattleBaseUnit* InUnit);
	
	UFUNCTION(BlueprintCallable)
	void SetActive(bool bActive);
	
protected:
	UPROPERTY(meta=(BindWidget)) class UImage* PortraitImage;
	UPROPERTY(meta=(BindWidget)) class UProgressBar* HPBar;
	UPROPERTY(meta=(BindWidget)) class UTextBlock* HPText;
	UPROPERTY(meta=(BindWidgetOptional)) class UBorder* ActiveBorder;
	
private:
	UPROPERTY() TObjectPtr<ABattleBaseUnit> Unit;
	
	UFUNCTION()
	void HandleHPChanged(float CurrentHP, float MaxHP);
	
	UFUNCTION()
	void HandleDied();
};
