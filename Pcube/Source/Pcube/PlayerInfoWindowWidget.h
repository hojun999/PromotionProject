// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerInfoWindowWidget.generated.h"

class UButton;
class UCanvasPanel;
class UImage;
class UTextBlock;

class UEquipmentSubsystem;
class UBattleInfoTransferSubsystem;
class UUnitDataAsset;
class UPlayerInfoEquipButtonWidget;
class UEquipPartsWindowWidget;
struct FPlayerInfoEquipmentButtonDef;

/**
 * PlayerInfoWindow (World UI)
 * - 파티원이 2명인 전제에 맞춰: 초상화 버튼(Btn_Ally0/1)로 선택
 * - 3D 프리뷰 대신 이미지(Img_Illustration)로 표시
 * - 스탯 + 무기부품 합산 효과(Proj/Hit/DmgMul 등) 표시
 */
UCLASS()
class PCUBE_API UPlayerInfoWindowWidget : public UUserWidget
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

	UFUNCTION(BlueprintImplementableEvent, Category="PlayerInfo")
	void BP_OpenWeaponModWindow(int32 PartyIndex);

	UFUNCTION(BlueprintImplementableEvent, Category="PlayerInfo")
	void BP_OpenArmorModWindow(int32 PartyIndex);

	// C++ 기본 구현이 실패했을 때 BP 커스텀 처리용 fallback
	UFUNCTION(BlueprintImplementableEvent, Category="PlayerInfo")
	void BP_OpenEquipmentPartsSelectionWindow(int32 PartyIndex, FName EquipmentKey);

private:
	UFUNCTION() void HandleAlly0Clicked();
	UFUNCTION() void HandleAlly1Clicked();
	UFUNCTION() void HandleWeaponSlotClicked();
	UFUNCTION() void HandleArmorSlotClicked();
	UFUNCTION() void HandleEquipmentChanged(int32 PartyIndex);
	UFUNCTION() void HandleEquipmentButtonClicked(FName EquipmentKey);

	void SelectPartyMember(int32 PartyIndex);
	void RefreshPortraitButtons();
	void RefreshSelectedMemberPanel();
	void RefreshStatsPanel();
	void RebuildEquipmentButtons();
	void EnsureDefaultWeaponsInitialized();
	void HideEquipPartsWindow();
	bool TryToggleEquipmentPartsWindow(FName EquipmentKey);
	bool ResolveEquipmentButtonDef(FName EquipmentKey, FPlayerInfoEquipmentButtonDef& OutDef) const;
	FVector2D GetEquipButtonWindowPosition(FName EquipmentKey) const;

	int32 GetPartyCount() const;
	UUnitDataAsset* GetPartyUnitData(int32 PartyIndex) const;

private:
	UPROPERTY(meta=(BindWidget))
	UButton* Btn_Ally0 = nullptr;

	UPROPERTY(meta=(BindWidgetOptional))
	UButton* Btn_Ally1 = nullptr;

	UPROPERTY(meta=(BindWidgetOptional))
	UImage* Img_Portrait0 = nullptr;

	UPROPERTY(meta=(BindWidgetOptional))
	UImage* Img_Portrait1 = nullptr;

	UPROPERTY(meta=(BindWidgetOptional))
	UImage* Img_Illustration = nullptr;

	UPROPERTY(meta=(BindWidgetOptional))
	UCanvasPanel* Canvas_EquipButtons = nullptr;

	UPROPERTY(meta=(BindWidgetOptional)) UTextBlock* Text_HP = nullptr;
	UPROPERTY(meta=(BindWidgetOptional)) UTextBlock* Text_SP = nullptr;
	UPROPERTY(meta=(BindWidgetOptional)) UTextBlock* Text_ATK = nullptr;
	UPROPERTY(meta=(BindWidgetOptional)) UTextBlock* Text_SPD = nullptr;
	UPROPERTY(meta=(BindWidgetOptional)) UTextBlock* Text_Effects = nullptr;
	UPROPERTY(meta=(BindWidgetOptional)) UTextBlock* Text_Proj = nullptr;
	UPROPERTY(meta=(BindWidgetOptional)) UTextBlock* Text_Hit = nullptr;
	UPROPERTY(meta=(BindWidgetOptional)) UTextBlock* Text_DmgMul = nullptr;

	UPROPERTY(meta=(BindWidgetOptional)) UButton* Btn_WeaponSlot = nullptr;
	UPROPERTY(meta=(BindWidgetOptional)) UButton* Btn_ArmorSlot = nullptr;
	UPROPERTY(meta=(BindWidgetOptional)) UImage* Img_WeaponIcon = nullptr;
	UPROPERTY(meta=(BindWidgetOptional)) UImage* Img_ArmorIcon = nullptr;

	UPROPERTY(EditAnywhere, Category="PlayerInfo|UI")
	TSubclassOf<UPlayerInfoEquipButtonWidget> EquipButtonWidgetClass;

	UPROPERTY(EditAnywhere, Category="PlayerInfo|UI")
	TSubclassOf<UEquipPartsWindowWidget> EquipPartsWindowClass;

private:
	UPROPERTY() TObjectPtr<UEquipmentSubsystem> EquipmentSubsystem = nullptr;
	UPROPERTY() TObjectPtr<UBattleInfoTransferSubsystem> TransferSubsystem = nullptr;
	UPROPERTY() TObjectPtr<UEquipPartsWindowWidget> EquipPartsWindow = nullptr;
	UPROPERTY() TMap<FName, TObjectPtr<UPlayerInfoEquipButtonWidget>> EquipButtonsByKey;

	int32 SelectedPartyIndex = 0;
};
