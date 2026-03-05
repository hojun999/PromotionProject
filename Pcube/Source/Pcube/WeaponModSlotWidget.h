#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WeaponModSlotWidget.generated.h"

class UButton;
class UImage;
class UTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponSlotSelected, FName, SlotId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponSlotUnequip, FName, SlotId);

UCLASS()
class PCUBE_API UWeaponModSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void Init(FName InSlotId, FText InDisplayName);
	void SetEquipped(class UWeaponPartDataAsset* Part);

	UPROPERTY(BlueprintAssignable) FOnWeaponSlotSelected OnSelected;
	UPROPERTY(BlueprintAssignable) FOnWeaponSlotUnequip OnUnequip;

protected:
	virtual void NativeConstruct() override;

private:
	UFUNCTION() void HandleSelect();
	UFUNCTION() void HandleUnequip();

	UPROPERTY(meta=(BindWidget)) UButton* Btn_Select = nullptr;
	UPROPERTY(meta=(BindWidgetOptional)) UButton* Btn_Unequip = nullptr;
	UPROPERTY(meta=(BindWidget)) UImage* Img_Icon = nullptr;
	UPROPERTY(meta=(BindWidgetOptional)) UTextBlock* Text_Name = nullptr;

	FName SlotId = NAME_None;
	UPROPERTY() TObjectPtr<UWeaponPartDataAsset> Equipped = nullptr;
};