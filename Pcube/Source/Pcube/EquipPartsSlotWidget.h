#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EquipPartsSlotWidget.generated.h"

class UButton;
class UImage;
class UTextBlock;
class UWeaponPartDataAsset;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEquipPartsSlotClicked, UWeaponPartDataAsset*, Part);

UCLASS()
class PCUBE_API UEquipPartsSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void Init(UWeaponPartDataAsset* InPart, int32 InQuantity);

	UPROPERTY(BlueprintAssignable)
	FOnEquipPartsSlotClicked OnClicked;

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

	UPROPERTY(meta=(BindWidgetOptional))
	UTextBlock* Text_Name = nullptr;

	UPROPERTY(meta=(BindWidgetOptional))
	UTextBlock* Text_Count = nullptr;

	UPROPERTY()
	TObjectPtr<UWeaponPartDataAsset> Part = nullptr;

	int32 Quantity = 0;
};
