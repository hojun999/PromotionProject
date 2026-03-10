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
	// InQuantity: 인벤토리 보유 수량. 0이면 비활성(어둡게) 표시.
	UFUNCTION(BlueprintCallable)
	void Init(UWeaponPartDataAsset* InPart, int32 InQuantity);

	UPROPERTY(BlueprintAssignable)
	FOnEquipPartsSlotClicked OnClicked;

protected:
	virtual void NativeConstruct() override;

	// 비활성 상태일 때 이미지에 적용할 불투명도 (0~1). BP에서 조정 가능.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Appearance", meta=(ClampMin="0.0", ClampMax="1.0"))
	float UnavailableOpacity = 0.35f;

private:
	UFUNCTION()
	void HandleClicked();

	void ApplyAvailability(bool bAvailable);

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
	bool bAvailable = false;
};
