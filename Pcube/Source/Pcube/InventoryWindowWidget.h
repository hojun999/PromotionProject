// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventoryWindowWidget.generated.h"

class UUniformGridPanel;
class UButton;
class UTextBlock;
class UInventorySlotWidget;
class UInventorySubsystem;

UCLASS()
class PCUBE_API UInventoryWindowWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void Open();

	UFUNCTION(BlueprintCallable)
	void Close();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	UFUNCTION()
	void HandleInventoryChanged();

	UFUNCTION()
	void HandlePrevPage();

	UFUNCTION()
	void HandleNextPage();

	UFUNCTION()
	void HandleCloseClicked();

	UFUNCTION()
	void HandleSlotClicked(int32 SlotIndex, class UItemDataAsset* Item);

	void RebuildGrid();
	void UpdatePageText();

private:
	UPROPERTY(meta=(BindWidget))
	UUniformGridPanel* Grid_Inventory = nullptr; // 4x5

	UPROPERTY(meta=(BindWidgetOptional))
	UButton* Btn_Prev = nullptr;

	UPROPERTY(meta=(BindWidgetOptional))
	UButton* Btn_Next = nullptr;

	UPROPERTY(meta=(BindWidgetOptional))
	UButton* Btn_Close = nullptr;

	UPROPERTY(meta=(BindWidgetOptional))
	UTextBlock* Text_Page = nullptr;

	UPROPERTY(EditAnywhere, Category="UI")
	TSubclassOf<UInventorySlotWidget> InventorySlotClass;

private:
	UPROPERTY()
	TObjectPtr<UInventorySubsystem> Inv = nullptr;

	int32 PageIndex = 0;

	static constexpr int32 Rows = 5;
	static constexpr int32 Cols = 4;
	static constexpr int32 SlotsPerPage = Rows * Cols;
};
