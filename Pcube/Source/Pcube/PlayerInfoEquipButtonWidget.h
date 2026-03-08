// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerInfoEquipButtonWidget.generated.h"

class UButton;
class UImage;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerInfoEquipButtonClicked, FName, EquipmentKey);

/**
 * Simple image button used inside PlayerInfoWindowWidget.
 * - Shows an icon (weapon/armor/etc)
 * - Emits EquipmentKey on click
 */
UCLASS()
class PCUBE_API UPlayerInfoEquipButtonWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void Init(FName InEquipmentKey, UTexture2D* InIcon);

	UPROPERTY(BlueprintAssignable)
	FOnPlayerInfoEquipButtonClicked OnClicked;

protected:
	virtual void NativeConstruct() override;

private:
	UFUNCTION()
	void HandleClicked();

private:
	UPROPERTY(meta=(BindWidget))
	UButton* Btn_Root = nullptr;

	UPROPERTY(meta=(BindWidgetOptional))
	UImage* Img_Icon = nullptr;

	FName EquipmentKey = NAME_None;
};
