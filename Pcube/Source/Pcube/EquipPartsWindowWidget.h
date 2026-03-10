#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UnitDataAsset.h"
#include "WeaponDataAsset.h"
#include "EquipPartsWindowWidget.generated.h"

class UArmorDataAsset;
class UUniformGridPanel;
class UCanvasPanel;
class UButton;
class UTextBlock;
class UImage;
class UEquipPartsSlotWidget;
class UEquipSlotWidget;
class UWeaponPartDataAsset;
class UEquipmentSubsystem;
class UInventorySubsystem;

UCLASS()
class PCUBE_API UEquipPartsWindowWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void OpenForWeapon(int32 InPartyIndex, FName InEquipmentButtonKey, UWeaponDataAsset* InWeapon);
	
	UFUNCTION(BlueprintCallable)
	void OpenForArmor(int32 InPartyIndex, FName InEquipmentButtonKey, UArmorDataAsset* InArmor);
	
	// UFUNCTION(BlueprintCallable)
	// void OpenForEquipment(int32 InPartyIndex, FName InEquipmentKey);

	UFUNCTION(BlueprintCallable)
	void Close();

	UFUNCTION(BlueprintCallable)
	void Refresh();

	UFUNCTION(BlueprintPure)
	int32 GetCurrentPartyIndex() const { return PartyIndex; }

	UFUNCTION(BlueprintPure)
	FName GetCurrentEquipmentButtonKey() const { return EquipmentButtonKey; }
	
	// UFUNCTION(BlueprintPure)
	// FName GetCurrentEquipmentKey() const { return EquipmentKey; }

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	UFUNCTION()
	void HandleCloseClicked();

	UFUNCTION()
	void HandleInventoryChanged();

	UFUNCTION()
	void HandleEquipmentChanged(int32 ChangedPartyIndex);

	// UFUNCTION()
	// void HandlePartSlotSelected(FName PartSlotKey);
	
	// UFUNCTION()
	// void HandlePartSlotSelected(UWeaponPartDataAsset* Part);

	// UFUNCTION()
	// void HandlePartSlotUnequip(FName PartSlotKey);
	
	UFUNCTION()
	void HandlePartCandidateClicked(UWeaponPartDataAsset* Part);
	
	//void RebuildSlotCanvas();
	void RebuildCandidateCanvas();
	void ClearCandidateCanvas();
	void ResetActiveEquipment();
	//void AutoSelectFirstSlotIfNeeded();
	const TArray<FWeaponModSlotDef>* GetActiveSlots() const;
	UTexture2D* GetActiveIllustration() const;
	FText GetActiveTitle() const;
	
	//void RebuildGrid();

private:
	UPROPERTY(meta=(BindWidget))
	UImage* Img_EquipmentIllustration = nullptr;

	
	
	UPROPERTY(meta=(BindWidgetOptional))
	UCanvasPanel* Canvas_Parts = nullptr;

	UPROPERTY(meta=(BindWidgetOptional))
	UButton* Btn_Close = nullptr;

	UPROPERTY(meta=(BindWidgetOptional))
	UTextBlock* Text_Title = nullptr;

	// UPROPERTY(EditAnywhere, Category="UI")
	// TSubclassOf<UEquipSlotWidget> EquipSlotWidgetClass;
	
	UPROPERTY(EditAnywhere, Category="UI")
	TSubclassOf<UEquipPartsSlotWidget> PartCandidateWidgetClass;
	
	UPROPERTY(EditAnywhere, Category="UI")
	FVector2D DefaultSlotWidgetSize = FVector2D(56.f, 56.f);
	
	UPROPERTY()
	TObjectPtr<UEquipmentSubsystem> EquipmentSubsystem = nullptr;

	UPROPERTY()
	TObjectPtr<UInventorySubsystem> InventorySubsystem = nullptr;

	UPROPERTY()
	TObjectPtr<UWeaponDataAsset> ActiveWeapon = nullptr;

	UPROPERTY()
	TObjectPtr<UArmorDataAsset> ActiveArmor = nullptr;
	
	int32 PartyIndex = INDEX_NONE;
	FName EquipmentButtonKey = NAME_None;
	//FName SelectedPartSlotKey = NAME_None;
	EPlayerInfoEquipmentKind CurrentEquipmentKind = EPlayerInfoEquipmentKind::Weapon;
	
	static constexpr int32 Rows = 2;
	static constexpr int32 Cols = 2;
	static constexpr int32 MaxSlots = Rows * Cols;
};
